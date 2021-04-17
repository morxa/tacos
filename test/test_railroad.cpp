/***************************************************************************
 *  test_railroad.cpp - A railroad example
 *
 *  Created:   Mon  1 Mar 17:18:27 CET 2021
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
 *  Read the full text in the LICENSE.md file.
 */

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_product.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "synchronous_product/search.h"
#include "synchronous_product/search_tree.h"
#include "visualization/tree_to_graphviz.h"

#include <catch2/catch_test_macros.hpp>

namespace {

using Location   = automata::ta::Location<std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;
using automata::AtomicClockConstraintT;
using automata::Time;
using F  = logic::MTLFormula<std::string>;
using AP = logic::AtomicProposition<std::string>;
using synchronous_product::NodeLabel;
using TreeSearch = synchronous_product::TreeSearch<std::vector<std::string>, std::string>;

std::tuple<automata::ta::TimedAutomaton<std::vector<std::string>, std::string>,
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
		// environment_actions.insert({});
		automata.push_back(
		  TA{{Location{"OPEN"}, Location{"CLOSING"}, Location{"CLOSED"}, Location{"OPENING"}},
		     {start_close, finish_close, start_open, finish_open},
		     Location{"OPEN"},
		     {Location{"OPEN"}, Location{"CLOSING"}, Location{"CLOSED"}, Location{"OPENING"}},
		     {clock},
		     {Transition(Location{"OPEN"}, start_close, Location{"CLOSING"}, {}, {clock}),
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
		                 {{clock, AtomicClockConstraintT<std::equal_to<Time>>(2)}},
		                 {clock})}});
		const std::string i_s = std::to_string(i);
		const auto        far = i == 1 ? Location{"FAR"} : Location{"BEHIND_" + std::to_string(i - 1)};
		const Location    near{"NEAR_" + i_s};
		const Location    in{"IN_" + i_s};
		const Location    behind{"BEHIND_" + i_s};
		train_locations.insert({far, near, in, behind});
		const std::string get_near{"get_near_" + i_s};
		const std::string enter{"enter_" + i_s};
		const std::string leave{"leave_" + i_s};
		train_actions.insert({get_near, enter, leave});
		train_transitions.push_back(
		  Transition{far,
		             get_near,
		             near,
		             {{"t", AtomicClockConstraintT<std::greater<Time>>(distances[i - 1])}},
		             {"t"}});
		train_transitions.push_back(
		  Transition{near, enter, in, {{"t", AtomicClockConstraintT<std::greater<Time>>(2)}}, {"t"}});
		train_transitions.push_back(Transition{
		  in, leave, behind, {{"t", AtomicClockConstraintT<std::equal_to<Time>>(1)}}, {"t"}});
	}
	automata.push_back(TA{train_locations,
	                      train_actions,
	                      Location{"FAR"},
	                      {Location{"BEHIND_" + std::to_string(distances.size())}},
	                      {"t"},
	                      train_transitions});
	environment_actions.insert(std::begin(train_actions), std::end(train_actions));
	return std::make_tuple(automata::ta::get_product(automata),
	                       controller_actions,
	                       environment_actions);
}

TEST_CASE("A single railroad crossing", "[railroad]")
{
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%t %v");
	const auto &[product, controller_actions, environment_actions] = create_crossing_problem({2});
	std::set<AP> actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               inserter(actions, end(actions)));
	const auto finish_close_1 = F{AP{"finish_close_1"}};
	const auto start_open_1   = F{AP{"start_open_1"}};
	const auto enter_1        = F{AP{"enter_1"}};
	auto       ata =
	  mtl_ata_translation::translate(((!finish_close_1).until(enter_1) && finally(enter_1)), actions);
	INFO("TA: " << product);
	INFO("ATA: " << ata);
	TreeSearch search{&product, &ata, controller_actions, environment_actions, 2, true, true};
	search.build_tree(true);
	INFO("Tree:\n" << synchronous_product::node_to_string(*search.get_root(), true));
#ifdef HAVE_VISUALIZATION
	visualization::search_tree_to_graphviz(*search.get_root(), true).render_to_file("railroad1.svg");
#endif
	CHECK(search.get_root()->label == NodeLabel::TOP);
}

TEST_CASE("Two railroad crossings", "[.medium][railroad]")
{
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%t %v");
	const auto &[product, controller_actions, environment_actions] = create_crossing_problem({2, 2});
	std::set<AP> actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               inserter(actions, end(actions)));
	const auto finish_close_1 = F{AP{"finish_close_1"}};
	const auto start_open_1   = F{AP{"start_open_1"}};
	const auto enter_1        = F{AP{"enter_1"}};
	const auto finish_close_2 = F{AP{"finish_close_2"}};
	const auto start_open_2   = F{AP{"start_open_2"}};
	const auto enter_2        = F{AP{"enter_2"}};
	auto       ata =
	  mtl_ata_translation::translate(((!finish_close_1).until(enter_1) && finally(enter_1))
	                                   || ((!finish_close_2).until(enter_2) && finally(enter_2)),
	                                 actions);
	INFO("TA: " << product);
	INFO("ATA: " << ata);
	TreeSearch search{&product, &ata, controller_actions, environment_actions, 2, true, true};
	search.build_tree(true);
	INFO("Tree:\n" << synchronous_product::node_to_string(*search.get_root(), true));
#ifdef HAVE_VISUALIZATION
	visualization::search_tree_to_graphviz(*search.get_root(), true).render_to_file("railroad2.svg");
#endif
	std::size_t size         = 0;
	std::size_t non_canceled = 0;
	for (auto it = synchronous_product::begin(search.get_root());
	     it != synchronous_product::end(search.get_root());
	     it++) {
		size++;
		if (it->label != synchronous_product::NodeLabel::CANCELED) {
			non_canceled++;
		}
	}
	INFO("Tree size: " << size);
	INFO("Non-canceled: " << non_canceled);
	CHECK(search.get_root()->label == NodeLabel::TOP);
}

TEST_CASE("Three railroad crossings", "[.large][railroad]")
{
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%t %v");
	const auto &[product, controller_actions, environment_actions] =
	  create_crossing_problem({2, 2, 2});
	std::set<AP> actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               inserter(actions, end(actions)));
	const auto finish_close_1 = F{AP{"finish_close_1"}};
	const auto start_open_1   = F{AP{"start_open_1"}};
	const auto enter_1        = F{AP{"enter_1"}};
	const auto finish_close_2 = F{AP{"finish_close_2"}};
	const auto start_open_2   = F{AP{"start_open_2"}};
	const auto enter_2        = F{AP{"enter_2"}};
	const auto finish_close_3 = F{AP{"finish_close_3"}};
	const auto start_open_3   = F{AP{"start_open_3"}};
	const auto enter_3        = F{AP{"enter_3"}};
	auto       ata =
	  mtl_ata_translation::translate(((!finish_close_1).until(enter_1) && finally(enter_1))
	                                   || ((!finish_close_2).until(enter_2) && finally(enter_2))
	                                   || ((!finish_close_3).until(enter_3) && finally(enter_3)),
	                                 actions);
	// INFO("TA: " << product);
	// INFO("ATA: " << ata);
	TreeSearch search{&product, &ata, controller_actions, environment_actions, 2, true, true};
	search.build_tree(true);
#ifdef HAVE_VISUALIZATION
	visualization::search_tree_to_graphviz(*search.get_root(), true).render_to_file("railroad3.svg");
#endif
	std::size_t size         = 0;
	std::size_t non_canceled = 0;
	for (auto it = synchronous_product::begin(search.get_root());
	     it != synchronous_product::end(search.get_root());
	     it++) {
		size++;
		if (it->label != synchronous_product::NodeLabel::CANCELED) {
			non_canceled++;
		}
	}
	INFO("Tree size: " << size);
	INFO("Non-canceled: " << non_canceled);
	CHECK(search.get_root()->label == NodeLabel::TOP);
	// INFO("Tree:\n" << synchronous_product::node_to_string(*search.get_root(), true));
}

} // namespace
