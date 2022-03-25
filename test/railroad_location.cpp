/***************************************************************************
 *  railroad.cpp - Utility functions for railroad test scenario
 *
 *  Created: Tue 20 Apr 2021 13:00:42 CEST 13:00
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "railroad_location.h"

#include "automata/ta_product.h"
#include "mtl/MTLFormula.h"
#include "utilities/Interval.h"
#include "visualization/ta_to_graphviz.h"

#include <spdlog/spdlog.h>

using namespace tacos;

using automata::Time;

using Location        = automata::ta::Location<std::string>;
using ProductLocation = automata::ta::Location<std::vector<std::string>>;
using TA              = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition      = automata::ta::Transition<std::string, std::string>;
using automata::AtomicClockConstraintT;
using F  = logic::MTLFormula<std::vector<std::string>>;
using AP = logic::AtomicProposition<std::vector<std::string>>;

F
create_disjunction(const std::vector<ProductLocation> &disjuncts)
{
	if (disjuncts.empty()) {
		return F::FALSE();
	}
	F disjunction = F{AP{disjuncts[0]}};
	std::for_each(next(begin(disjuncts)), end(disjuncts), [&disjunction](auto &&disjunct) {
		disjunction = disjunction || F{AP{disjunct}};
	});
	return disjunction;
}

std::tuple<automata::ta::TimedAutomaton<std::vector<std::string>, std::string>,
           logic::MTLFormula<std::vector<std::string>>,
           std::set<std::string>,
           std::set<std::string>>
create_crossing_problem(std::vector<Time> distances)
{
	std::vector<TA>         automata;
	std::set<std::string>   controller_actions;
	std::set<std::string>   environment_actions;
	std::set<std::string>   train_actions;
	std::set<Location>      train_locations;
	std::vector<Transition> train_transitions;
	for (std::size_t i = 1; i <= distances.size(); ++i) {
		const std::string clock        = "c_" + std::to_string(i);
		const std::string start_close  = "start_close_" + std::to_string(i);
		const std::string finish_close = "finish_close_" + std::to_string(i);
		const std::string start_open   = "start_open_" + std::to_string(i);
		const std::string finish_open  = "finish_open_" + std::to_string(i);
		controller_actions.insert({start_close, start_open, finish_close, finish_open});
		automata.push_back(
		  TA{{Location{"OPEN"}, Location{"CLOSING"}, Location{"CLOSED"}, Location{"OPENING"}},
		     {start_close, finish_close, start_open, finish_open},
		     Location{"OPEN"},
		     {Location{"OPEN"}, Location{"CLOSING"}, Location{"CLOSED"}, Location{"OPENING"}},
		     {clock},
		     {
		       Transition(Location{"OPEN"}, start_close, Location{"CLOSING"}, {}, {clock}),
		       Transition(Location{"CLOSING"},
		                  finish_close,
		                  Location{"CLOSED"},
		                  {{clock, AtomicClockConstraintT<std::equal_to<Time>>(1)}},
		                  {clock}),
		       Transition(Location{"CLOSED"},
		                  start_open,
		                  Location{"OPENING"},
		                  {{clock, AtomicClockConstraintT<std::greater_equal<Time>>(1)}},
		                  {clock}),
		       Transition(Location{"OPENING"},
		                  finish_open,
		                  Location{"OPEN"},
		                  {{clock, AtomicClockConstraintT<std::equal_to<Time>>(1)}},
		                  {clock}),
		     }});
		const std::string i_s = std::to_string(i);
		const auto     far = i == 1 ? Location{"FAR"} : Location{"FAR_BEHIND_" + std::to_string(i - 1)};
		const Location near{"NEAR_" + i_s};
		const Location in{"IN_" + i_s};
		const Location behind{"BEHIND_" + i_s};
		const Location far_behind{"FAR_BEHIND_" + i_s};
		train_locations.insert({far, near, in, behind, far_behind});
		const std::string get_near{"get_near_" + i_s};
		const std::string enter{"enter_" + i_s};
		const std::string leave{"leave_" + i_s};
		const std::string travel{"travel_" + i_s};
		train_actions.insert({get_near, enter, leave, travel});
		train_transitions.push_back(
		  Transition{far,
		             get_near,
		             near,
		             {{"t", AtomicClockConstraintT<std::equal_to<Time>>(distances[i - 1])}},
		             {"t"}});
		train_transitions.push_back(
		  Transition{near,
		             enter,
		             in,
		             {{"t", AtomicClockConstraintT<std::greater_equal<Time>>(0)},
		              {"t", AtomicClockConstraintT<std::less_equal<Time>>(1)}},
		             {"t"}});
		train_transitions.push_back(Transition{
		  in, leave, behind, {{"t", AtomicClockConstraintT<std::equal_to<Time>>(1)}}, {"t"}});
		train_transitions.push_back(Transition{
		  behind, travel, far_behind, {{"t", AtomicClockConstraintT<std::equal_to<Time>>(2)}}, {"t"}});
	}
	automata.push_back(TA{train_locations,
	                      train_actions,
	                      Location{"FAR"},
	                      {Location{"FAR_BEHIND_" + std::to_string(distances.size())}},
	                      {"t"},
	                      train_transitions});
	environment_actions.insert(std::begin(train_actions), std::end(train_actions));
	for (std::size_t i = 1; i < automata.size(); i++) {
		tacos::visualization::ta_to_graphviz(automata[i - 1])
		  .render_to_file(fmt::format("railroad{}_crossing_{}.pdf", distances.size(), i));
	}
	tacos::visualization::ta_to_graphviz(automata.back())
	  .render_to_file(fmt::format("railroad{}_train.pdf", distances.size()));

	auto                         product_automaton = automata::ta::get_product(automata);
	std::vector<ProductLocation> bad_locations;
	std::map<std::size_t, std::vector<ProductLocation>> in_locations;
	std::map<std::size_t, std::vector<ProductLocation>> open_locations;
	for (const auto &location : product_automaton.get_locations()) {
		for (std::size_t i = 0; i < distances.size(); ++i) {
			if (location.get()[i] == "OPEN") {
				open_locations[i].push_back(location);
			}
			if (location.get().back() == "IN_" + std::to_string(i + 1)) {
				in_locations[i].push_back(location);
				if (location.get()[i] != "CLOSED") {
					bad_locations.push_back(location);
				}
			}
		}
	}
	auto spec = logic::finally(create_disjunction(bad_locations));
	for (std::size_t i = 0; i < distances.size(); ++i) {
		spec =
		  spec
		  || globally(!create_disjunction(open_locations[i]) && !create_disjunction(in_locations[i])
		              && !finally(create_disjunction(in_locations[i]), logic::TimeInterval(0, 1)));
	}

	return std::make_tuple(product_automaton, spec, controller_actions, environment_actions);
}
