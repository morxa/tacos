/***************************************************************************
 *  test_ta_print.cpp - Tests for printing a TA
 *
 *  Created:   Tue  9 Feb 10:22:59 CET 2021
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

#include "automata/automata.h"
#include "automata/ta.h"

#include <catch2/catch.hpp>
#include <sstream>

namespace {

using TA            = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition    = automata::ta::Transition<std::string, std::string>;
using Configuration = automata::ta::Configuration<std::string>;

TEST_CASE("Print a TA transition", "[ta][print]")
{
	SECTION("A transition without constraints")
	{
		std::stringstream str;
		str << Transition("s0", "a", "s1");
		CHECK(str.str() == u8"s0 → a / ⊤ / {} → s1");
	}
	SECTION("A transition with a constraint")
	{
		std::stringstream str;
		str << Transition("s0",
		                  "a",
		                  "s1",
		                  {{"x", automata::AtomicClockConstraintT<std::less<automata::Time>>{1}}});
		CHECK(str.str() == u8"s0 → a / x < 1 / {} → s1");
	}
	SECTION("A transition with two constraints")
	{
		std::stringstream str;
		str << Transition("s0",
		                  "a",
		                  "s1",
		                  {{"x", automata::AtomicClockConstraintT<std::less<automata::Time>>{1}},
		                   {"y", automata::AtomicClockConstraintT<std::greater<automata::Time>>{2}}});
		CHECK(str.str() == u8"s0 → a / x < 1 ∧ y > 2 / {} → s1");
	}
	SECTION("A transition with a constraint and a reset")
	{
		std::stringstream str;
		str << Transition("s0",
		                  "a",
		                  "s1",
		                  {{"x", automata::AtomicClockConstraintT<std::less<automata::Time>>{1}}},
		                  {"x"});
		CHECK(str.str() == u8"s0 → a / x < 1 / { x } → s1");
	}
}

TEST_CASE("Print a TA", "[ta][print]")
{
	TA ta{{"a"}, "s0", {"s1"}};
	ta.add_clock("x");
	ta.add_transition(
	  Transition("s0",
	             "a",
	             "s0",
	             {{"x", automata::AtomicClockConstraintT<std::greater<automata::Time>>(2)}},
	             {"x"}));
	ta.add_transition(
	  Transition("s0",
	             "a",
	             "s1",
	             {{"x", automata::AtomicClockConstraintT<std::less<automata::Time>>(2)}},
	             {"x"}));
	std::stringstream str;
	str << ta;
	CHECK(str.str()
	      == "Alphabet: { a }, initial location: s0, final locations: { s1 }, transitions:\n"
	         "s0 → a / x > 2 / { x } → s0\n"
	         "s0 → a / x < 2 / { x } → s1\n");
}

TEST_CASE("Print a TA configuration", "[ta][print]")
{
	SECTION("A configuration without a clock")
	{
		std::stringstream str;
		str << Configuration{"s0", {}};
		CHECK(str.str() == "(s0, {})");
	}
	SECTION("A configuration with a single clock")
	{
		std::stringstream str;
		str << Configuration{"s0", {{"x", 1}}};
		CHECK(str.str() == "(s0, { x: 1 } )");
	}
	SECTION("A configuration with two clocks")
	{
		std::stringstream str;
		str << Configuration{"s0", {{"c1", 1}, {"c2", 3}}};
		CHECK(str.str() == "(s0, { c1: 1, c2: 3 } )");
	}
}

} // namespace
