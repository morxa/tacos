/***************************************************************************
 *  test_golog_search.cpp - Search over a Golog++ program
 *
 *  Created:   Tue 19 Oct 14:32:54 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

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
#include <stdexcept>
#include <string>

namespace {

using namespace tacos;
using namespace logic;
using search::GologConfiguration;
using search::GologLocation;
using search::GologProgram;
using search::LabelReason;
using search::NodeLabel;

using AP = AtomicProposition<std::string>;

TEST_CASE("Initialize Golog programs", "[golog]")
{
	CHECK_THROWS_AS(GologProgram{R"(action say() { })"}, std::invalid_argument);

	GologProgram program(R"(
    action say() { }
    procedure main() { say(); }
  )");
	const auto   initial_configuration = program.get_initial_configuration();
	CHECK(initial_configuration.clock_valuations == ClockSetValuation{{"golog", 0}});
}

TEST_CASE("Get the currently satisfied Golog fluents", "[golog]")
{
	GologProgram program(R"(
    symbol domain location = { aachen, wien }
    bool fluent visited(symbol l) {
      initially:
        (l) = false;
    }
    action visit(symbol l) {
      effect:
        visited(l) = true;
    }
    procedure main() { visit(aachen); visit(wien); }
  )",
	                     {"visited(aachen)", "visited(wien)"});
	auto         history = program.get_empty_history();
	CHECK(program.get_satisfied_fluents(*history) == std::set<std::string>{});
	// start visit(aachen)
	auto successor = program.get_semantics().trans_all(*history)[0];
	auto remaining = std::get<1>(successor);
	history        = std::get<2>(successor);
	// end visit(aachen)
	successor =
	  program.get_semantics().trans_all(*history, remaining.get(), {{"visit(aachen)", 0}})[0];
	remaining = std::get<1>(successor);
	history   = std::get<2>(successor);
	CHECK(program.get_satisfied_fluents(*history) == std::set<std::string>{"visited(aachen)"});
	// start visit(wien)
	successor = program.get_semantics().trans_all(*history, remaining.get())[0];
	remaining = std::get<1>(successor);
	history   = std::get<2>(successor);
	// end visit(wien)
	successor = program.get_semantics().trans_all(*history,
	                                              remaining.get(),
	                                              {{"visit(aachen)", 0}, {"visit(wien)", 0}})[0];
	remaining = std::get<1>(successor);
	history   = std::get<2>(successor);
	CHECK(program.get_satisfied_fluents(*history)
	      == std::set<std::string>{"visited(aachen)", "visited(wien)"});
}

TEST_CASE("Compare GologLocations", "[golog]")
{
	GologProgram        program(R"(
    action say() { }
    procedure main() { say(); }
  )");
	const GologLocation l1{&program, program.get_empty_program(), program.get_empty_history()};
	const GologLocation l2{&program, program.get_empty_program(), program.get_empty_history()};
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
	CHECK(program.is_accepting_configuration(GologConfiguration{
	  GologLocation{&program, program.get_empty_program(), program.get_empty_history()}, {}}));
	CHECK(program.is_accepting_configuration(GologConfiguration{
	  GologLocation{&program,
	                std::make_shared<gologpp::ManagedTerm>(gologpp::make_ec_list({})),
	                program.get_empty_history()},
	  {}}));
}

TEST_CASE("Search on a simple golog program", "[golog][search]")
{
	spdlog::set_level(spdlog::level::trace);
	GologProgram                program(R"(
    action say() {}
    action yell() {}
    action hear() {}
    procedure main() { hear(); choose { yell(); say(); } }
  )");
	const auto                  f = finally(MTLFormula<std::string>{std::string{"start(yell())"}});
	const std::set<std::string> controller_actions = {
	  "start(hear())",
	  "start(say())",
	  "start(yell())",
	};
	const std::set<std::string> environment_actions = {
	  "end(hear())",
	  "end(say())",
	  "end(yell())",
	};
	std::set<std::string> actions;
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
	auto ata = mtl_ata_translation::translate(f, action_aps);
	CAPTURE(ata);
	search::TreeSearch<search::GologLocation, std::string, std::string, false, GologProgram> search(
	  &program, &ata, controller_actions, environment_actions, 1, true, false);
	search.build_tree(false);
	visualization::search_tree_to_graphviz(*search.get_root()).render_to_file("golog_tree.png");

	//  4 for start(hear()).
	CHECK(search.get_root()->get_children().size() == 4);

	// First child, reached with (1, start(hear())).
	const auto c1 =
	  search.get_root()->get_children().at(std::make_pair(0, std::string{"start(hear())"}));
	const auto c2 =
	  search.get_root()->get_children().at(std::make_pair(1, std::string{"start(hear())"}));
	const auto c3 =
	  search.get_root()->get_children().at(std::make_pair(2, std::string{"start(hear())"}));
	const auto c4 =
	  search.get_root()->get_children().at(std::make_pair(3, std::string{"start(hear())"}));
	// Each start(hear()) with a different time increment leads to a different node, as the ATA state
	// is different.
	CHECK(std::set{{c1, c2, c3, c4}}.size() == 4);

	// 4 for end(hear()).
	CHECK(c1->get_children().size() == 4);

	// start(hear()) -> end(hear())
	const auto     c1c1 = c1->get_children().at(std::make_pair(0, std::string{"end(hear())"}));
	const auto     c1c2 = c1->get_children().at(std::make_pair(1, std::string{"end(hear())"}));
	const auto     c1c3 = c1->get_children().at(std::make_pair(2, std::string{"end(hear())"}));
	const auto     c1c4 = c1->get_children().at(std::make_pair(3, std::string{"end(hear())"}));
	const std::set c1children{{c1c1, c1c2, c1c3, c1c4}};
	// Each end(hear()) with a different time increment leads to a different node, as the ATA state is
	// different.
	CHECK(c1children.size() == 4);
	for (const auto &c1child : c1children) {
		CHECK(c1child->label == NodeLabel::TOP);
	}

	// 4 for start(say()), 4 for start(yell()).
	CHECK(c1c1->get_children().size() == 8);
	for (unsigned int inc = 0; inc <= 3; ++inc) {
		CHECK(c1c1->get_children().find(std::make_pair(inc, std::string{"start(say())"}))
		      != c1c1->get_children().end());
	}
	for (unsigned int inc = 0; inc <= 3; ++inc) {
		CHECK(c1c1->get_children().find(std::make_pair(inc, std::string{"start(yell())"}))
		      != c1c1->get_children().end());
	}

	// start(hear()) -> end(hear()) -> start(say())
	const auto c1c1c1 = c1c1->get_children().at(std::make_pair(0, std::string{"start(say())"}));
	CHECK(c1c1c1->label == NodeLabel::TOP);
	CHECK(c1c1c1->label_reason == LabelReason::NO_BAD_ENV_ACTION);

	// start(hear()) -> end(hear()) -> start(yell())
	const auto c1c1c2 = c1c1->get_children().at(std::make_pair(0, std::string{"start(yell())"}));
	CHECK(c1c1c2->label == NodeLabel::BOTTOM);
	CHECK(c1c1c2->label_reason == LabelReason::BAD_ENV_ACTION_FIRST);

	// 4 for end(say()).
	CHECK(c1c1c1->get_children().size() == 4);

	// start(hear()) -> end(hear()) -> start(say()) -> end(say())
	const auto c1c1c1c1 = c1c1c1->get_children().at(std::make_pair(0, std::string{"end(say())"}));
	CHECK(c1c1c1c1->label == NodeLabel::TOP);
	CHECK(c1c1c1c1->label_reason == LabelReason::DEAD_NODE);
	CHECK(c1c1c1c1->get_children().empty());

	// 4 for end(yell()).
	CHECK(c1c1c2->get_children().size() == 4);

	// start(hear()) -> end(hear()) -> start(yell()) -> end(yell())
	const auto c1c1c2c1 = c1c1c2->get_children().at(std::make_pair(0, std::string{"end(yell())"}));
	CHECK(c1c1c2c1->label == NodeLabel::BOTTOM);
	CHECK(c1c1c2c1->label_reason == LabelReason::BAD_NODE);
	CHECK(c1c1c2c1->get_children().empty());

	// Overall search result.
	CHECK(search.get_root()->label == NodeLabel::TOP);
}

TEST_CASE("Search on fluent constraints", "[golog][search]")
{
	const auto spec =
	  globally(!MTLFormula<std::string>{logic::AtomicProposition<std::string>{"visited(wien)"}});
	auto ata = mtl_ata_translation::translate<std::string, std::set<std::string>, true>(spec);
	// TODO refactor so we do not need unwrap anymore
	auto unwrap = [](std::set<AtomicProposition<std::set<std::string>>> input) {
		std::set<std::string> res;
		for (const auto &i : input) {
			for (const auto &s : i.ap_) {
				res.insert(s);
			}
		}
		return res;
	};

	GologProgram program(R"(
    symbol domain location = { aachen, wien }
    bool fluent visited(symbol l) {
      initially:
        (l) = false;
    }
    action visit(symbol l) {
      effect:
        visited(l) = true;
    }
    procedure main() { visit(aachen); visit(wien); }
  )",
	                     unwrap(ata.get_alphabet()));
	CAPTURE(ata);
	const std::set<std::string> controller_actions = {
	  "start(visit(aachen))",
	  "start(visit(wien))",
	};
	const std::set<std::string> environment_actions = {
	  "end(visit(aachen))",
	  "end(visit(wien))",
	};
	search::TreeSearch<search::GologLocation, std::string, std::string, true, GologProgram, true>
	  search(&program, &ata, controller_actions, environment_actions, 0, false, false);
	search.build_tree(false);
	search.label();
	visualization::search_tree_to_graphviz(*search.get_root())
	  .render_to_file("golog_fluent_search.svg");
	CHECK(search.get_root()->label == NodeLabel::TOP);
	// 2 for start(visit(aachen)).
	CHECK(search.get_root()->get_children().size() == 2);
	const auto c1 =
	  search.get_root()->get_children().at(std::make_pair(0, std::string{"start(visit(aachen))"}));
	CHECK(c1->label == NodeLabel::TOP);
	// 2 for end(visit(aachen)).
	CHECK(c1->get_children().size() == 2);

	const auto c2 =
	  search.get_root()->get_children().at(std::make_pair(1, std::string{"start(visit(aachen))"}));
	CHECK(c2->label == NodeLabel::TOP);
	// 2 for end(visit(aachen)).
	CHECK(c2->get_children().size() == 2);

	const auto c1c1 = c1->get_children().at(std::make_pair(0, std::string{"end(visit(aachen))"}));
	CAPTURE(*c1c1);
	CHECK(c1c1->label == NodeLabel::TOP);
	CHECK(c1c1->get_children().size() == 2);

	const auto c1c2 = c1->get_children().at(std::make_pair(1, std::string{"end(visit(aachen))"}));
	CAPTURE(*c1c2);
	CHECK(c1c2->label == NodeLabel::TOP);
	// Only 1 for end(visit(aachen)) because all regions are maxed out already, so the time successors
	// with increment 0 and increment 1 are the same.
	CHECK(c1c2->get_children().size() == 1);

	const auto c2c1 = c2->get_children().at(std::make_pair(0, std::string{"end(visit(aachen))"}));
	CAPTURE(*c2c1);
	CHECK(c2c1->label == NodeLabel::TOP);
	// 2 for end(visit(aachen)).
	CHECK(c2c1->get_children().size() == 2);

	const auto c2c2 = c2->get_children().at(std::make_pair(1, std::string{"end(visit(aachen))"}));
	CAPTURE(*c2c2);
	CHECK(c2c2->label == NodeLabel::TOP);
	// Only 1 for end(visit(aachen)) because all regions are maxed out already, so the time successors
	// with increment 0 and increment 1 are the same.
	CHECK(c2c2->get_children().size() == 1);
}
} // namespace
