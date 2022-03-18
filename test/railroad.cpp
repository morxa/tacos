/***************************************************************************
 *  railroad.cpp - Utility functions for railroad test scenario
 *
 *  Created: Tue 20 Apr 2021 13:00:42 CEST 13:00
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "railroad.h"

#include "automata/ta_product.h"
#include "mtl/MTLFormula.h"
#include "utilities/Interval.h"
#include "visualization/ta_to_graphviz.h"

using namespace tacos;

using Location   = automata::ta::Location<std::string_view>;
using TA         = automata::ta::TimedAutomaton<std::string_view, std::string_view>;
using Transition = automata::ta::Transition<std::string_view, std::string_view>;
using automata::AtomicClockConstraintT;
using F  = logic::MTLFormula<std::string_view>;
using AP = logic::AtomicProposition<std::string_view>;

RailroadProblem::RailroadProblem(std::vector<tacos::Time> distances) : spec(F::TRUE())
{
	std::vector<TA>            automata;
	std::set<std::string_view> train_actions;
	std::set<Location>         train_locations;
	std::vector<Transition>    train_transitions;
	std::vector<F>             spec_disjuncts;
	Location                   train_init;
	Location                   train_final;
	for (std::size_t i = 1; i <= distances.size(); ++i) {
		const std::string      clock       = "c_" + std::to_string(i);
		const std::string_view start_close = *actions.insert("start_close_" + std::to_string(i)).first;
		const std::string_view finish_close =
		  *actions.insert("finish_close_" + std::to_string(i)).first;
		const std::string_view start_open  = *actions.insert("start_open_" + std::to_string(i)).first;
		const std::string_view finish_open = *actions.insert("finish_open_" + std::to_string(i)).first;
		controller_actions.insert({start_close, finish_close, start_open, finish_open});
		const Location open    = Location{*locations.insert("OPEN").first};
		const Location closing = Location{*locations.insert("CLOSING").first};
		const Location closed  = Location{*locations.insert("CLOSED").first};
		const Location opening = Location{*locations.insert("OPENING").first};
		automata.push_back(
		  TA{{open, closing, closed, opening},
		     {start_close, finish_close, start_open, finish_open},
		     open,
		     {open, closing, closed, opening},
		     {clock},
		     {
		       Transition(open, start_close, closing, {}, {clock}),
		       Transition(closing,
		                  finish_close,
		                  closed,
		                  {{clock, AtomicClockConstraintT<std::equal_to<Time>>(1)}},
		                  {clock}),
		       Transition(closed,
		                  start_open,
		                  opening,
		                  {{clock, AtomicClockConstraintT<std::greater_equal<Time>>(1)}},
		                  {clock}),
		       Transition(opening,
		                  finish_open,
		                  open,
		                  {{clock, AtomicClockConstraintT<std::equal_to<Time>>(1)}},
		                  {clock}),
		     }});
		const Location far =
		  i == 1 ? Location{*locations.insert("FAR").first}
		         : Location{*locations.insert("FAR_BEHIND_" + std::to_string(i - 1)).first};
		if (i == 1) {
			train_init = far;
		}
		const std::string i_s  = std::to_string(i);
		const Location    near = Location{*locations.insert("NEAR_" + i_s).first};
		const Location    in{*locations.insert("IN_" + i_s).first};
		const Location    behind{*locations.insert("BEHIND_" + i_s).first};
		const Location    far_behind{*locations.insert("FAR_BEHIND_" + i_s).first};
		if (i == distances.size()) {
			train_final = far_behind;
		}
		train_locations.insert({far, near, in, behind, far_behind});
		std::string_view get_near{*actions.insert("get_near_" + i_s).first};
		std::string_view enter{*actions.insert("enter_" + i_s).first};
		std::string_view leave{*actions.insert("leave_" + i_s).first};
		std::string_view travel{*actions.insert("travel_" + i_s).first};
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
		const auto finish_close_f = F{AP{finish_close}};
		const auto start_open_f   = F{AP{start_open}};
		const auto finish_open_f  = F{AP{finish_open}};
		const auto enter_f        = F{AP{enter}};
		const auto leave_f        = F{AP{leave}};
		const auto travel_f       = F{AP{travel}};
		spec_disjuncts.push_back(enter_f.dual_until(!finish_close_f)
		                         || start_open_f.dual_until(!leave_f)
		                         || travel_f.dual_until(!finish_open_f));
	}
	automata.push_back(
	  TA{train_locations, train_actions, train_init, {train_final}, {"t"}, train_transitions});
	environment_actions.insert(std::begin(train_actions), std::end(train_actions));
	spec = spec_disjuncts[0];
	std::for_each(std::next(std::begin(spec_disjuncts)),
	              std::end(spec_disjuncts),
	              [this](auto &&spec_disjunct) { spec = spec || spec_disjunct; });
	for (std::size_t i = 1; i < automata.size(); i++) {
		visualization::ta_to_graphviz(automata[i - 1])
		  .render_to_file(fmt::format("railroad{}_crossing_{}.pdf", distances.size(), i));
	}
	visualization::ta_to_graphviz(automata.back())
	  .render_to_file(fmt::format("railroad{}_train.pdf", distances.size()));

	plant.reset(new automata::ta::TimedAutomaton<std::vector<std::string_view>, std::string_view>(
	  automata::ta::get_product(automata)));
}
