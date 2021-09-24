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

#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "parser/parser.h"
#include "search/golog_adapter.h"
#include "semantics/readylog/execution.h"

#include <semantics/readylog/history.h>
#include <semantics/readylog/utilities.h>

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <variant>

namespace {

using gologpp::parser::parse_string;

using tacos::search::get_next_canonical_words;
using namespace tacos::logic;

using CanonicalABWord = tacos::search::CanonicalABWord<tacos::search::GologLocation, std::string>;

class GologTestFixture
{
public:
	GologTestFixture()
	{
		gologpp::eclipse_opts options;
		options.trace    = false;
		options.toplevel = false;
		options.guitrace = true;
		gologpp::ReadylogContext::init(options);
		semantics = &gologpp::ReadylogContext::instance().semantics_factory();
		history.reset(new gologpp::History());
	}

	virtual ~GologTestFixture()
	{
		history.reset();
		gologpp::global_scope().clear();
		gologpp::ReadylogContext::shutdown();
	}

protected:
	void
	init_program(const std::string &program)
	{
		if (initialized) {
			throw std::logic_error("Cannot reinitialize the program");
		}
		initialized = true;
		parse_string(program);
		main_proc = gologpp::global_scope().lookup_global<gologpp::Procedure>("main");
		REQUIRE(main_proc != nullptr);
		main = main_proc->ref({});
		main->attach_semantics(*semantics);
		gologpp::global_scope().implement_globals(*semantics, gologpp::ReadylogContext::instance());
		history->attach_semantics(*semantics);
	}
	gologpp::SemanticsFactory *           semantics;
	std::shared_ptr<gologpp::Procedure>   main_proc;
	gologpp::Instruction *                main;
	gologpp::shared_ptr<gologpp::History> history;

private:
	bool initialized = false;
};

TEST_CASE_METHOD(GologTestFixture, "Golog successors", "[golog]")
{
	init_program(R"(
    action say() { }
    procedure main() { say(); }
  )");
	const auto f = finally(MTLFormula<std::string>{std::string{"end(say())"}});
	const auto ata =
	  tacos::mtl_ata_translation::translate(f,
	                                        {std::string{"start(say())"}, std::string{"end(say())"}});
	CAPTURE(ata);
	tacos::search::GologConfiguration golog_configuration;
	golog_configuration.location.remaining_program.reset(
	  new gologpp::ManagedTerm(main->semantics().plterm()));
	golog_configuration.location.history = history;
	golog_configuration.clock_valuations.insert(std::make_pair(std::string{"golog"}, tacos::Clock{}));
	const auto next_words = get_next_canonical_words(
	  main->semantics(), ata, {golog_configuration, ata.get_initial_configuration()}, 2);
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
