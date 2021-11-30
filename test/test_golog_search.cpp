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
using tacos::search::LabelReason;
using tacos::search::NodeLabel;

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
    procedure main() { hear(); choose { yell(); say(); } }
  )");
	const auto   f = finally(MTLFormula<std::string>{std::string{"start(yell())"}})
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
	    search(&program, &ata, controller_actions, environment_actions, 1, true, false);
	search.build_tree(false);
	tacos::visualization::search_tree_to_graphviz(*search.get_root())
	  .render_to_file("golog_tree.png");

	CHECK(search.get_nodes().size() == 11);
	// 4 for start(hear()) and 1 for env_terminate.
	CHECK(search.get_root()->get_children().size() == 5);

	// First child, reached with (1, start(hear())).
	const auto c1 =
	  search.get_root()->get_children().at(std::make_pair(0, std::string{"start(hear())"}));
	// start(hear()) always leads to the same node, no matter what the time increment is.
	CHECK(search.get_root()->get_children().at(std::make_pair(1, std::string{"start(hear())"}))
	      == c1);
	CHECK(search.get_root()->get_children().at(std::make_pair(2, std::string{"start(hear())"}))
	      == c1);
	CHECK(search.get_root()->get_children().at(std::make_pair(3, std::string{"start(hear())"}))
	      == c1);

	// Child reached from root with (3, env_terminate).
	const auto cterm =
	  search.get_root()->get_children().at(std::make_pair(3, std::string{"env_terminate"}));
	CHECK(cterm->get_children().empty());
	CHECK(cterm->label == NodeLabel::BOTTOM);
	CHECK(cterm->label_reason == LabelReason::BAD_NODE);

	// c1 and cterm are the only children of root.
	for (const auto &[_, child] : search.get_root()->get_children()) {
		CHECK((child == c1 || child == cterm));
	}

	// 4 for end(hear()) and 1 for ctl_terminate.
	CHECK(c1->get_children().size() == 5);

	// start(hear()) -> end(hear())
	const auto c1c1 = c1->get_children().at(std::make_pair(0, std::string{"end(hear())"}));
	CHECK(c1c1->label == NodeLabel::TOP);
	// end(hear()) always leads to the same node, no matter what the time increment is.
	CHECK(c1->get_children().at(std::make_pair(1, std::string{"end(hear())"})) == c1c1);
	CHECK(c1->get_children().at(std::make_pair(2, std::string{"end(hear())"})) == c1c1);
	CHECK(c1->get_children().at(std::make_pair(3, std::string{"end(hear())"})) == c1c1);

	// start(hear()) -> ctl_terminate
	const auto c1cterm = c1->get_children().at(std::make_pair(3, std::string{"ctl_terminate"}));
	CHECK(c1cterm->get_children().empty());
	CHECK(c1cterm->label == NodeLabel::TOP);
	CHECK(c1cterm->label_reason == LabelReason::DEAD_NODE);

	// c1c1 and c1cterm are the only children of c1.
	for (const auto &[_, child] : c1->get_children()) {
		CHECK((child == c1c1 || child == c1cterm));
	}

	// 4 for start(say()), 4 for start(yell()), 1 for env_terminate.
	CHECK(c1c1->get_children().size() == 9);

	// start(hear()) -> end(hear()) -> start(say())
	const auto c1c1c1 = c1c1->get_children().at(std::make_pair(0, std::string{"start(say())"}));
	CHECK(c1c1c1->label == NodeLabel::TOP);
	CHECK(c1c1c1->label_reason == LabelReason::NO_BAD_ENV_ACTION);

	// start(hear()) -> end(hear()) -> start(yell())
	const auto c1c1c2 = c1c1->get_children().at(std::make_pair(0, std::string{"start(yell())"}));
	CHECK(c1c1c2->label == NodeLabel::BOTTOM);
	CHECK(c1c1c2->label_reason == LabelReason::BAD_ENV_ACTION_FIRST);

	// start(hear()) -> end(hear()) -> env_terminate
	const auto c1c1cterm = c1c1->get_children().at(std::make_pair(3, std::string{"env_terminate"}));
	CHECK(c1c1cterm->get_children().empty());
	CHECK(c1c1cterm->label == NodeLabel::BOTTOM);
	CHECK(c1c1cterm->label_reason == LabelReason::BAD_NODE);

	// c1c1c1, c1c1c2, and c1c1cterm are the only children of c1c1.
	for (const auto &[_, child] : c1c1->get_children()) {
		CHECK((child == c1c1c1 || child == c1c1c2 || child == c1c1cterm));
	}

	// 4 for end(say()) and 1 for ctl_terminate.
	CHECK(c1c1c1->get_children().size() == 5);

	// start(hear()) -> end(hear()) -> start(say()) -> end(say())
	const auto c1c1c1c1 = c1c1c1->get_children().at(std::make_pair(0, std::string{"end(say())"}));
	CHECK(c1c1c1c1->label == NodeLabel::TOP);
	CHECK(c1c1c1c1->label_reason == LabelReason::DEAD_NODE);
	CHECK(c1c1c1c1->get_children().empty());

	// start(hear()) -> end(hear()) -> start(say()) -> ctl_terminate
	const auto c1c1c1cterm =
	  c1c1c1->get_children().at(std::make_pair(3, std::string{"ctl_terminate"}));
	CHECK(c1c1c1cterm->get_children().empty());
	CHECK(c1c1c1cterm->label == NodeLabel::TOP);
	CHECK(c1c1c1cterm->label_reason == LabelReason::DEAD_NODE);

	// 4 for end(yell()) and 1 for ctl_terminate.
	CHECK(c1c1c2->get_children().size() == 5);

	// start(hear()) -> end(hear()) -> start(yell()) -> end(yell())
	const auto c1c1c2c1 = c1c1c2->get_children().at(std::make_pair(0, std::string{"end(yell())"}));
	CHECK(c1c1c2c1->label == NodeLabel::BOTTOM);
	CHECK(c1c1c2c1->label_reason == LabelReason::BAD_NODE);
	CHECK(c1c1c2c1->get_children().empty());

	// start(hear()) -> end(hear()) -> start(yell()) -> ctl_terminate
	const auto c1c1c2cterm =
	  c1c1c2->get_children().at(std::make_pair(3, std::string{"ctl_terminate"}));
	CHECK(c1c1c2cterm->get_children().empty());
	// The node is already bad because we have violated F start(yell()) already.
	CHECK(c1c1c2cterm->label == NodeLabel::BOTTOM);
	CHECK(c1c1c2cterm->label_reason == LabelReason::BAD_NODE);

	// Overall search result.
	CHECK(search.get_root()->label == NodeLabel::TOP);
}

} // namespace
