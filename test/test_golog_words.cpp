/***************************************************************************
 *  test_golog_words.cpp - Test successor word generation for Golog
 *
 *  Created:   Tue 21 Sep 13:52:54 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "gocos/golog_adapter.h"
#include "gocos/golog_program.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"

#include <parser/parser.h>
#include <semantics/readylog/execution.h>
#include <semantics/readylog/history.h>
#include <semantics/readylog/utilities.h>

#include <catch2/catch_test_macros.hpp>
#include <string>

namespace {

using namespace tacos;
using namespace logic;

using search::get_next_canonical_words;
using search::GologProgram;

using CanonicalABWord = search::CanonicalABWord<search::GologLocation, std::string>;

TEST_CASE("Golog successors", "[golog]")
{
	GologProgram program(R"(
    action say() { }
    procedure main() { say(); }
  )");
	const auto   f = finally(MTLFormula<std::string>{std::string{"end(say())"}});
	const auto   ata =
	  mtl_ata_translation::translate(f, {std::string{"start(say())"}, std::string{"end(say())"}});
	CAPTURE(ata);
	search::GologConfiguration  golog_configuration = program.get_initial_configuration();
	const std::set<std::string> controller_actions  = {"start(say())"};
	const std::set<std::string> environment_actions = {"end(say())"};
	const auto                  next_words =
	  get_next_canonical_words<GologProgram, std::string, std::string, false>(controller_actions,
	                                                                          environment_actions)(
	    program, ata, {golog_configuration, ata.get_initial_configuration()}, 0, 2);
	CAPTURE(next_words);
	REQUIRE(next_words.size() == 1);
	CHECK(next_words.find("start(say())") != std::end(next_words));
	CHECK(next_words.begin()->first == "start(say())");
	CHECK(next_words.begin()->second.size() == 1);
	// 1 for the plant configuration, 1 for the ATA configuration
	CHECK(next_words.begin()->second.begin()->size() == 2);
	for (const auto &ab_symbol : next_words.begin()->second.front()) {
		using GologSymbol = search::PlantRegionState<search::GologLocation>;
		if (std::holds_alternative<GologSymbol>(ab_symbol)) {
			const auto &golog_symbol = std::get<GologSymbol>(ab_symbol);
			CHECK(golog_symbol.location.history->special_semantics().as_transitions().size() == 1);
			CHECK(gologpp::ReadylogContext::instance().to_string(*golog_symbol.location.remaining_program)
			      == "[end('gpp~say')]");
			CHECK(golog_symbol.clock == "say()");
			CHECK(golog_symbol.region_index == 0);
		} else {
			REQUIRE(std::holds_alternative<search::ATARegionState<std::string>>(ab_symbol));
			const auto &ata_symbol = std::get<search::ATARegionState<std::string>>(ab_symbol);
			CHECK(ata_symbol.formula == f);
			CHECK(ata_symbol.region_index == 0);
		}
	}
}
TEST_CASE("Golog fluent-based successors", "[golog]")
{
	using get_next_canonical_words =
	  get_next_canonical_words<GologProgram, std::string, std::string, true, true>;
	GologProgram program(R"(
    bool fluent said() {
      initially:
        () = false;
    }
    action say() {
      start_effect:
        said() = true;
    }
    procedure main() { say(); }
  )",
	                     {"said()"});
	const auto   f   = MTLFormula<std::string>{std::string{"said()"}};
	const auto   ata = mtl_ata_translation::translate<std::string, std::set<std::string>, true>(f);
	CAPTURE(ata);
	search::GologConfiguration  golog_configuration = program.get_initial_configuration();
	const std::set<std::string> controller_actions  = {"start(say())"};
	const std::set<std::string> environment_actions = {"end(say())"};
	const auto next_words = get_next_canonical_words(controller_actions, environment_actions)(
	  program, ata, {golog_configuration, ata.get_initial_configuration()}, 0, 2);
	CAPTURE(next_words);
	REQUIRE(next_words.size() == 1);
	CHECK(next_words.find("start(say())") != std::end(next_words));
	CHECK(next_words.begin()->first == "start(say())");
	CHECK(next_words.begin()->second.size() == 1);
	// There is no ATA state anymore.
	const auto &ab_symbols = next_words.begin()->second.front();
	CAPTURE(ab_symbols);
	REQUIRE(ab_symbols.size() == 1);
	const auto &ab_symbol = *ab_symbols.begin();
	using GologSymbol     = search::PlantRegionState<search::GologLocation>;
	REQUIRE(std::holds_alternative<GologSymbol>(ab_symbol));
	const auto &golog_symbol = std::get<GologSymbol>(ab_symbol);
	CHECK(golog_symbol.location.history->special_semantics().as_transitions().size() == 1);
	CHECK(golog_symbol.location.satisfied_fluents == std::set<std::string>{"said()"});
	CHECK(gologpp::ReadylogContext::instance().to_string(*golog_symbol.location.remaining_program)
	      == "[end('gpp~say')]");
	CHECK(golog_symbol.clock == "say()");
	CHECK(golog_symbol.region_index == 0);
}

} // namespace
