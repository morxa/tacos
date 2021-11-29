/***************************************************************************
 *  test_golog_search.cpp - Search over a Golog++ program
 *
 *  Created:   Tue 19 Oct 14:32:54 CEST 2021
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

#include "gocos/golog_adapter.h"
#include "gocos/golog_program.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "visualization/tree_to_graphviz.h"

#include <parser/parser.h>
#include <semantics/readylog/execution.h>
#include <semantics/readylog/history.h>
#include <semantics/readylog/utilities.h>
#include <spdlog/spdlog.h>

#include <catch2/catch_test_macros.hpp>
#include <string>

namespace {

using namespace tacos::logic;
using tacos::search::GologConfiguration;
using tacos::search::GologLocation;
using tacos::search::GologProgram;

using AP = AtomicProposition<std::string>;

TEST_CASE("Compare GologLocations", "[golog]")
{
	GologProgram        program(R"(
    action say() { }
    procedure main() { say(); }
  )");
	const GologLocation l1{program.get_empty_program(), program.get_empty_history()};
	const GologLocation l2{program.get_empty_program(), program.get_empty_history()};
	CHECK(!(l1 < l2));
	CHECK(!(l2 < l1));
	const auto i1 = program.get_initial_location();
	const auto i2 = program.get_initial_location();
	CHECK(!(i1 < i2));
	CHECK(!(i2 < i1));
	CHECK(l1 < i1);
	CHECK(l1 < i2);
	CHECK(l2 < i1);
	CHECK(l2 < i2);
}

TEST_CASE("Check Golog final locations", "[golog]")
{
	GologProgram program(R"(
    action say() { }
    procedure main() { say(); }
  )");

	CHECK(!program.is_accepting_configuration(program.get_initial_configuration()));
	CHECK(program.is_accepting_configuration(
	  GologConfiguration{GologLocation{program.get_empty_program(), program.get_empty_history()},
	                     {}}));
	CHECK(program.is_accepting_configuration(GologConfiguration{
	  GologLocation{std::make_shared<gologpp::ManagedTerm>(gologpp::make_ec_list({})),
	                program.get_empty_history()},
	  {}}));
}

TEST_CASE("Search on a simple golog program", "[golog][search]")
{
	spdlog::set_level(spdlog::level::trace);
	GologProgram program(R"(
    action say() { }
    action yell() { }
    action hear() { }
    procedure main() { hear(); choose { say(); yell(); } }
  )");
	const auto   f = finally(MTLFormula<std::string>{std::string{"start(say())"}})
	               || finally(MTLFormula<std::string>{std::string{"env_terminate"}})
	               || MTLFormula<std::string>{std::string{"env_terminate"}};
	const std::set<std::string> controller_actions  = {"start(hear())",
                                                    "start(say())",
                                                    "start(yell())",
                                                    "ctl_terminate"};
	const std::set<std::string> environment_actions = {"end(hear())",
	                                                   "end(say())",
	                                                   "end(yell())",
	                                                   "env_terminate"};
	std::set<std::string>       actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               std::inserter(actions, actions.end()));
	std::set<AP> action_aps;
	std::transform(begin(actions),
	               end(actions),
	               std::inserter(action_aps, end(action_aps)),
	               [](const auto &action) { return AP{action}; });
	auto ata = tacos::mtl_ata_translation::translate(f, action_aps);
	CAPTURE(ata);
	tacos::search::
	  TreeSearch<tacos::search::GologLocation, std::string, std::string, false, GologProgram>
	    search(&program, &ata, controller_actions, environment_actions, 1, true, true);
	search.build_tree(false);
	tacos::visualization::search_tree_to_graphviz(*search.get_root())
	  .render_to_file("golog_tree.png");

	CHECK(search.get_root()->label == tacos::search::NodeLabel::TOP);
}

} // namespace
