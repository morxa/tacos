/***************************************************************************
 *  benchmark_robot.cpp - Control a robot's camera
 *
 *  Created:   Mon 26 Jul 18:48:06 CEST 2021
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

#include "automata/automata.h"
#include "automata/ta_product.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "search/create_controller.h"
#include "search/search.h"
#include "visualization/interactive_tree_to_graphviz.h"
#include "visualization/ta_to_graphviz.h"
#include "visualization/tree_to_graphviz.h"

#include <catch2/catch_test_macros.hpp>

namespace {

using Search     = search::TreeSearch<std::vector<std::string>, std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using MTLFormula = logic::MTLFormula<std::string>;
using AP         = logic::AtomicProposition<std::string>;
using automata::AtomicClockConstraintT;

std::unique_ptr<
  search::Heuristic<long, search::SearchTreeNode<std::vector<std::string>, std::string>>>
generate_heuristic(long                  weight_canonical_words     = 0,
                   long                  weight_environment_actions = 0,
                   std::set<std::string> environment_actions        = {},
                   long                  weight_time_heuristic      = 1)
{
	using H = search::Heuristic<long, search::SearchTreeNode<std::vector<std::string>, std::string>>;
	std::vector<std::pair<long, std::unique_ptr<H>>> heuristics;
	heuristics.emplace_back(weight_canonical_words,
	                        std::make_unique<search::NumCanonicalWordsHeuristic<
	                          long,
	                          search::SearchTreeNode<std::vector<std::string>, std::string>>>());
	heuristics.emplace_back(weight_environment_actions,
	                        std::make_unique<search::PreferEnvironmentActionHeuristic<
	                          long,
	                          search::SearchTreeNode<std::vector<std::string>, std::string>,
	                          std::string>>(environment_actions));
	heuristics.emplace_back(
	  weight_time_heuristic,
	  std::make_unique<
	    search::TimeHeuristic<long,
	                          search::SearchTreeNode<std::vector<std::string>, std::string>>>());
	return std::make_unique<
	  search::CompositeHeuristic<long,
	                             search::SearchTreeNode<std::vector<std::string>, std::string>>>(
	  std::move(heuristics));
}

TEST_CASE("Logistics Robot Benchmark", "[.benchmark]")
{
	TA robot(
	  {
	    TA::Location{"AT-OUTPUT"},
	    TA::Location{"PICKED"},
	    TA::Location{"AT-DELIVERY"},
	    TA::Location{"PUT"},
	    TA::Location{"MOVING-TO-OUTPUT"},
	    TA::Location{"MOVING-TO-DELIVERY"},
	  },
	  {"move", "arrive", "pick", "put"},
	  TA::Location{"MOVING-TO-OUTPUT"},
	  {TA::Location{"AT-OUTPUT"}},
	  {"c-travel", "cp"},
	  {
	    TA::Transition(TA::Location{"PICKED"}, "move", TA::Location{"MOVING-TO-DELIVERY"}),
	    TA::Transition(TA::Location{"PUT"}, "move", TA::Location{"MOVING-TO-OUTPUT"}),
	    TA::Transition(TA::Location{"MOVING-TO-DELIVERY"},
	                   "arrive",
	                   TA::Location{"AT-DELIVERY"},
	                   {{"c-travel", AtomicClockConstraintT<std::equal_to<automata::Time>>{3}}},
	                   {"c-travel", "cp"}),
	    TA::Transition(TA::Location{"MOVING-TO-OUTPUT"},
	                   "arrive",
	                   TA::Location{"AT-OUTPUT"},
	                   {{"c-travel", AtomicClockConstraintT<std::equal_to<automata::Time>>{3}}},
	                   {"c-travel", "cp"}),
	    TA::Transition(TA::Location{"AT-OUTPUT"},
	                   "pick",
	                   TA::Location{"PICKED"},
	                   {{"cp", AtomicClockConstraintT<std::equal_to<automata::Time>>{1}}}),
	    TA::Transition(TA::Location{"AT-DELIVERY"},
	                   "put",
	                   TA::Location{"PUT"},
	                   {{"cp", AtomicClockConstraintT<std::equal_to<automata::Time>>{1}}}),
	  });
	TA camera(
	  {TA::Location{"CAMERA-OFF"}, TA::Location{"CAMERA-ON"}},
	  {"switch-on", "switch-off"},
	  TA::Location{"CAMERA-OFF"},
	  {TA::Location{"CAMERA-OFF"}},
	  {"c-camera"},
	  {TA::Transition(TA::Location{"CAMERA-OFF"},
	                  "switch-on",
	                  TA::Location{"CAMERA-ON"},
	                  {{"c-camera", AtomicClockConstraintT<std::greater_equal<automata::Time>>{1}}},
	                  {"c-camera"}),
	   TA::Transition(TA::Location{"CAMERA-ON"},
	                  "switch-off",
	                  TA::Location{"CAMERA-OFF"},
	                  {{"c-camera", AtomicClockConstraintT<std::greater_equal<automata::Time>>{1}},
	                   {"c-camera", AtomicClockConstraintT<std::less_equal<automata::Time>>{4}}},
	                  {"c-camera"})});
	auto product = automata::ta::get_product<std::string, std::string>({robot, camera});
	CAPTURE(product);
	const MTLFormula pick{AP{"pick"}};
	const MTLFormula camera_on{AP{"switch-on"}};
	const MTLFormula camera_off{AP{"switch-off"}};
	auto             spec = finally(finally(pick, logic::TimeInterval(0, 1)).dual_until(!camera_on));
	// finally(pick, logic::TimeInterval(0, 2)).dual_until(camera_on && !camera_off.until(pick);
	auto ata = mtl_ata_translation::translate(
	  spec, {AP{"pick"}, AP{"put"}, AP{"move"}, AP{"arrive"}, AP{"switch-on"}, AP{"switch-off"}});
	CAPTURE(ata);
	// TODO add check if the actions actually exist
	Search search(&product,
	              &ata,
	              {"switch-on", "switch-off"},
	              {"move", "arrive", "pick", "put"},
	              4,
	              true,
	              true,
	              generate_heuristic(12, 10, {"move", "arrive", "pick", "put"}));
	search.build_tree(true);
	// visualization::search_tree_to_graphviz_interactive(search.get_root(),
	//                                                   "rcll_search_interactive.svg");
	auto controller = controller_synthesis::create_controller(search.get_root(),
	                                                          {"switch-on", "switch-off"},
	                                                          {"move", "arrive", "pick", "put"},
	                                                          4);
	CAPTURE(controller);
	visualization::ta_to_graphviz(controller, false).render_to_file("rcll_controller.svg");
}
} // namespace
