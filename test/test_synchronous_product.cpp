/***************************************************************************
 *  test_synchronous_product.cpp - Test synchronous products
 *
 *  Created:   Mon 21 Dec 16:56:08 CET 2020
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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
#include "automata/ta.h"
#include "mtl/MTLFormula.h"

#include <synchronous_product/synchronous_product.h>

#include <catch2/catch.hpp>
#include <sstream>
#include <variant>

namespace {

using automata::ClockValuation;
using automata::ta::ClockSetValuation;
using synchronous_product::ABRegionSymbol;
using synchronous_product::ATAConfiguration;
using synchronous_product::ATARegionState;
using synchronous_product::get_canonical_word;
using synchronous_product::TARegionState;

TEST_CASE("Print a TARegionState", "[print]")
{
	const TARegionState<std::string> state{"s", "c", 1};
	std::stringstream                stream;
	stream << state;
	REQUIRE(stream.str() == "(s, c, 1)");
}

TEST_CASE("Print an ATARegionState", "[print]")
{
	const ATARegionState<std::string> state{logic::MTLFormula<std::string>{
	                                          logic::AtomicProposition<std::string>{"s"}},
	                                        2};
	std::stringstream                 stream;
	stream << state;
	REQUIRE(stream.str() == "(s, 2)");
}

TEST_CASE("Print an ABRegionSymbol", "[print]")
{
	{
		const ABRegionSymbol<std::string, std::string> symbol = TARegionState<std::string>("s", "c", 1);
		std::stringstream                              stream;
		stream << symbol;
		REQUIRE(stream.str() == "(s, c, 1)");
	}
	{
		const ABRegionSymbol<std::string, std::string> symbol = ATARegionState<std::string>(
		  logic::MTLFormula<std::string>{logic::AtomicProposition<std::string>{"s"}}, 2);
		std::stringstream stream;
		stream << symbol;
		REQUIRE(stream.str() == "(s, 2)");
	}
}

TEST_CASE("Print a set of ABRegionSymbols (one Abs_i)", "[print]")
{
	{
		const std::set<ABRegionSymbol<std::string, std::string>> word;
		std::stringstream                                        stream;
		stream << word;
		CHECK(stream.str() == "{}");
	}
	{
		std::set<ABRegionSymbol<std::string, std::string>> word{
		  TARegionState<std::string>("s", "c", 1),
		  ATARegionState<std::string>(
		    logic::MTLFormula<std::string>{logic::AtomicProposition<std::string>{"s"}}, 2)};
		std::stringstream stream;
		stream << word;
		CHECK(stream.str() == "{ (s, c, 1), (s, 2) }");
	}
}

TEST_CASE("Print the canonical word H(s)", "[print]")
{
	{
		const std::vector<std::set<ABRegionSymbol<std::string, std::string>>> word;
		std::stringstream                                                     stream;
		stream << word;
		CHECK(stream.str() == "[]");
	}
	{
		std::vector<std::set<ABRegionSymbol<std::string, std::string>>> word;
		word.push_back(std::set<ABRegionSymbol<std::string, std::string>>(
		  {TARegionState<std::string>("s", "c", 1),
		   ATARegionState<std::string>(
		     logic::MTLFormula<std::string>{logic::AtomicProposition<std::string>{"s"}}, 2)}));
		{
			std::stringstream stream;
			stream << word;
			CHECK(stream.str() == "[ { (s, c, 1), (s, 2) } ]");
		}
		word.push_back(std::set<ABRegionSymbol<std::string, std::string>>(
		  {TARegionState<std::string>("s", "c2", 5),
		   ATARegionState<std::string>(
		     logic::MTLFormula<std::string>{logic::AtomicProposition<std::string>{"a"}}, 3)}));
		{
			std::stringstream stream;
			stream << word;
			CHECK(stream.str() == "[ { (s, c, 1), (s, 2) }, { (s, c2, 5), (a, 3) } ]");
		}
		word.push_back(std::set<ABRegionSymbol<std::string, std::string>>(
		  {TARegionState<std::string>("s2", "c3", 10)}));
		{
			std::stringstream stream;
			stream << word;
			CHECK(stream.str() == "[ { (s, c, 1), (s, 2) }, { (s, c2, 5), (a, 3) }, { (s2, c3, 10) } ]");
		}
	}
}

TEST_CASE("Get a canonical word of an empty", "[canonical_word]")
{
	auto w = get_canonical_word<std::string, std::string>({}, {}, 3);
	INFO("Canonical word: " << w);
	REQUIRE(w.size() == 0);
}

TEST_CASE("Get a canonical word of a simple state", "[canonical_word]")
{
	const logic::MTLFormula                        f{logic::AtomicProposition<std::string>{"a"}};
	const ATAConfiguration<std::string>            ata_configuration = {{f, 0.0}};
	const ClockSetValuation                        v{{"c", 0}};
	const automata::ta::Configuration<std::string> ta_configuration{"s", v};
	const auto                                     w =
	  get_canonical_word<std::string, std::string>(ta_configuration, ata_configuration, 5);
	INFO("Canonical word: " << w);
	REQUIRE(w.size() == 1);
	const auto &abs1 = *w.begin();
	REQUIRE(abs1.size() == 2);
	const auto &symbol1 = *abs1.begin();
	REQUIRE(std::holds_alternative<TARegionState<std::string>>(symbol1));
	CHECK(std::get<TARegionState<std::string>>(symbol1) == TARegionState<std::string>("s", "c", 0));
	const auto &symbol2 = *std::next(abs1.begin());
	REQUIRE(std::holds_alternative<ATARegionState<std::string>>(symbol2));
	CHECK(std::get<ATARegionState<std::string>>(symbol2) == ATARegionState<std::string>(f, 0));
}

TEST_CASE("Get a canonical word of a more complex state", "[canonical_word]")
{
	const logic::MTLFormula                        a{logic::AtomicProposition<std::string>{"a"}};
	const logic::MTLFormula                        b{logic::AtomicProposition<std::string>{"b"}};
	const ATAConfiguration<std::string>            ata_configuration = {{a, 0.5}, {b, 1.5}};
	const ClockSetValuation                        v{{"c1", 0.1}, {"c2", 0.5}};
	const automata::ta::Configuration<std::string> ta_configuration{"s", v};
	const auto                                     w =
	  get_canonical_word<std::string, std::string>(ta_configuration, ata_configuration, 3);
	INFO("Canonical word: " << w);
	REQUIRE(w.size() == 2);
	{
		const auto &abs1 = *w.begin();
		REQUIRE(abs1.size() == 1);
		const auto &symbol1 = *abs1.begin();
		REQUIRE(std::holds_alternative<TARegionState<std::string>>(symbol1));
		CHECK(std::get<TARegionState<std::string>>(symbol1)
		      == TARegionState<std::string>("s", "c1", 1));
	}
	{
		const auto &abs2 = *std::next(w.begin());
		REQUIRE(abs2.size() == 3);
		const auto &symbol1 = *abs2.begin();
		REQUIRE(std::holds_alternative<TARegionState<std::string>>(symbol1));
		CHECK(std::get<TARegionState<std::string>>(symbol1)
		      == TARegionState<std::string>("s", "c2", 1));
		const auto &symbol2 = *std::next(abs2.begin());
		REQUIRE(std::holds_alternative<ATARegionState<std::string>>(symbol2));
		CHECK(std::get<ATARegionState<std::string>>(symbol2) == ATARegionState<std::string>(a, 1));
		const auto &symbol3 = *std::next(abs2.begin(), 2);
		REQUIRE(std::holds_alternative<ATARegionState<std::string>>(symbol3));
		CHECK(std::get<ATARegionState<std::string>>(symbol3) == ATARegionState<std::string>(b, 3));
	}
}

} // namespace
