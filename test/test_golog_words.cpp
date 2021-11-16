/***************************************************************************
 *  test_golog_words.cpp - Test successor word generation for Golog
 *
 *  Created:   Tue 21 Sep 13:52:54 CEST 2021
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
#include "parser/parser.h"
#include "semantics/readylog/execution.h"

#include <semantics/readylog/history.h>
#include <semantics/readylog/utilities.h>

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <variant>

namespace {

using tacos::search::get_next_canonical_words;
using tacos::search::GologProgram;
using namespace tacos::logic;

using CanonicalABWord = tacos::search::CanonicalABWord<tacos::search::GologLocation, std::string>;

TEST_CASE("Golog successors", "[golog]")
{
	GologProgram program(R"(
    action say() { }
    procedure main() { say(); }
  )");
	const auto   f = finally(MTLFormula<std::string>{std::string{"end(say())"}});
	const auto   ata =
	  tacos::mtl_ata_translation::translate(f,
	                                        {std::string{"start(say())"}, std::string{"end(say())"}});
	CAPTURE(ata);
	tacos::search::GologConfiguration golog_configuration = program.get_initial_configuration();
	const auto next_words = get_next_canonical_words<GologProgram, std::string, std::string, false>()(
	  program, ata, {golog_configuration, ata.get_initial_configuration()}, 0, 2);
	CAPTURE(next_words);
	CHECK(next_words.size() == 1);
	CHECK(next_words.find("start(say())") != std::end(next_words));
	CHECK(next_words.begin()->first == "start(say())");
	CHECK(next_words.begin()->second.size() == 1);
	for (const auto &ab_symbol : next_words.begin()->second.front()) {
		using GologSymbol = tacos::search::PlantRegionState<tacos::search::GologLocation>;
		if (std::holds_alternative<GologSymbol>(ab_symbol)) {
			const auto &golog_symbol = std::get<GologSymbol>(ab_symbol);
			CHECK(golog_symbol.location.history->special_semantics().as_transitions().size() == 1);
			CHECK(gologpp::ReadylogContext::instance().to_string(*golog_symbol.location.remaining_program)
			      == "[end('gpp~say')]");
			CHECK(golog_symbol.clock == "golog");
			CHECK(golog_symbol.region_index == 0);
		} else {
			REQUIRE(std::holds_alternative<tacos::search::ATARegionState<std::string>>(ab_symbol));
			const auto &ata_symbol = std::get<tacos::search::ATARegionState<std::string>>(ab_symbol);
			CHECK(ata_symbol.formula == f);
			CHECK(ata_symbol.region_index == 0);
		}
	}
}

} // namespace
