/***************************************************************************
 *  test_golog_robot.cpp - Test robot scenario with Golog program
 *
 *  Created:   Fri 28 Jan 11:44:37 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#include "gocos/golog_adapter.h"
#include "gocos/golog_program.h"
#include "golog_robot.h"
#include "heuristics_generator.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "search/create_controller.h"
#include "search/heuristics.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "visualization/interactive_tree_to_graphviz.h"
#include "visualization/ta_to_graphviz.h"
#include "visualization/tree_to_graphviz.h"

#include <catch2/catch_test_macros.hpp>

namespace {

using namespace tacos;
using logic::AtomicProposition;
using search::GologProgram;
using TreeSearch = tacos::search::
  TreeSearch<search::GologLocation, std::string, std::string, true, GologProgram, true>;

TEST_CASE("Test robot scenario with Golog", "[robot][golog]")
{
	const unsigned int camtime = 2;
	const auto [program_string, spec, controller_actions, environment_actions] =
	  create_robot_problem(camtime, false);

	CAPTURE(program_string);
	CAPTURE(spec);
	CAPTURE(controller_actions);
	CAPTURE(environment_actions);
	auto ata = tacos::mtl_ata_translation::translate<std::string, std::set<std::string>, true>(spec);
	CAPTURE(ata);
	auto unwrap = [](std::set<AtomicProposition<std::set<std::string>>> input) {
		std::set<std::string> res;
		for (const auto &i : input) {
			for (const auto &s : i.ap_) {
				res.insert(s);
			}
		}
		return res;
	};
	const auto relevant_fluents = unwrap(ata.get_alphabet());
	CAPTURE(relevant_fluents);
	GologProgram program(program_string, relevant_fluents, false, {"start(boot_camera())"});
	TreeSearch   search(&program,
                    &ata,
                    controller_actions,
                    environment_actions,
                    camtime,
                    true,
                    true,
                    std::make_unique<search::DfsHeuristic<long, TreeSearch::Node>>());
	// generate_heuristic<TreeSearch::Node>(16, 4, environment_actions, 1));
	search.build_tree(false);
	search.label();
	// visualization::search_tree_to_graphviz(*search.get_root(), true)
	//   .render_to_file("robot_golog.svg");
	CHECK(search.get_root()->label == search::NodeLabel::TOP);
	visualization::ta_to_graphviz(controller_synthesis::create_controller(search.get_root(),
	                                                                      controller_actions,
	                                                                      environment_actions,
	                                                                      camtime),
	                              false)
	  .render_to_file(fmt::format("robot_golog_controller_{}.pdf", camtime));
	// visualization::search_tree_to_graphviz_interactive(search.get_root(), "robot_search.png");
	//  CHECK(false);
}

} // namespace
