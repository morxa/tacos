/***************************************************************************
 *  test_golog_household.cpp - Test the household example
 *
 *  Created:   Wed 19 Oct 19:47:33 CEST 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "gocos/golog_adapter.h"
#include "gocos/golog_program.h"
#include "golog_household.h"
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

TEST_CASE("Test household scenario with Golog", "[golog]")
{
	const RegionIndex align_time = 1;
	const auto [program_string, spec, controller_actions, environment_actions] =
	  create_household_problem(align_time);

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
	GologProgram program(program_string, relevant_fluents, false, {"end(align(table))"});
	TreeSearch   search(&program,
                    &ata,
                    controller_actions,
                    environment_actions,
                    align_time,
                    true,
                    true,
                    std::make_unique<search::DfsHeuristic<long, TreeSearch::Node>>());
	// generate_heuristic<TreeSearch::Node>(16, 4, environment_actions, 1));
	search.build_tree(false);
	search.label();
	CHECK(search.get_root()->label == search::NodeLabel::TOP);
	visualization::ta_to_graphviz(controller_synthesis::create_controller(search.get_root(),
	                                                                      controller_actions,
	                                                                      environment_actions,
	                                                                      align_time),
	                              false)
	  .render_to_file("household_controller.svg");
	// visualization::search_tree_to_graphviz_interactive(search.get_root(), "robot_search.png");
	//  CHECK(false);
}

} // namespace
