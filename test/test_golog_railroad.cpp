/***************************************************************************
 *  test_golog_railroad.cpp - Test railroad scenario with Golog program
 *
 *  Created:   Fri 28 Jan 11:44:37 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "golog_adapter/golog_adapter.h"
#include "golog_adapter/golog_program.h"
#include "golog_railroad.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "search/create_controller.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "visualization/ta_to_graphviz.h"
#include "visualization/tree_to_graphviz.h"

#include <catch2/catch_test_macros.hpp>

namespace {

using namespace tacos;
using logic::AtomicProposition;
using search::GologProgram;
using TreeSearch = tacos::search::
  TreeSearch<search::GologLocation, std::string, std::string, true, GologProgram, true>;

TEST_CASE("Test railroad scenario with a golog program", "[railroad][.golog]")
{
	const auto [program_string, spec, controller_actions, environment_actions] =
	  create_crossing_problem({1});

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
	GologProgram program(program_string, unwrap(ata.get_alphabet()));
	TreeSearch   search(&program, &ata, controller_actions, environment_actions, 1, true, false);
	search.build_tree(false);
	search.label();
	visualization::search_tree_to_graphviz(*search.get_root()).render_to_file("railroad_golog.svg");
	REQUIRE(search.get_root()->label == search::NodeLabel::TOP);
	visualization::ta_to_graphviz(controller_synthesis::create_controller(
	                                search.get_root(), controller_actions, environment_actions, 1),
	                              false)
	  .render_to_file("railroad_golog_controller.svg");
}
} // namespace
