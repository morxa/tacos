/***************************************************************************
 *  test_ta.cpp - Test TA implementation
 *
 *  Created: Tue 26 May 2020 13:51:10 CEST 13:51
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
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include <ta/ta.h>

#include <catch2/catch.hpp>
#include <functional>

using namespace automata;
using namespace automata::ta;

TEST_CASE("Clock constraints with integers", "[ta]")
{
	using LT = AtomicClockConstraintT<std::less<Time>>;
	using LE = AtomicClockConstraintT<std::less_equal<Time>>;
	using EQ = AtomicClockConstraintT<std::equal_to<Time>>;
	using GE = AtomicClockConstraintT<std::greater_equal<Time>>;
	using GT = AtomicClockConstraintT<std::greater<Time>>;
	REQUIRE(LT(1).is_satisfied(0));
	REQUIRE(!LT(1).is_satisfied(1));
	REQUIRE(!LT(1).is_satisfied(2));
	REQUIRE(LE(1).is_satisfied(0));
	REQUIRE(LE(1).is_satisfied(1));
	REQUIRE(!LE(1).is_satisfied(2));
	REQUIRE(!EQ(1).is_satisfied(0));
	REQUIRE(EQ(1).is_satisfied(1));
	REQUIRE(!EQ(1).is_satisfied(2));
	REQUIRE(!GE(1).is_satisfied(0));
	REQUIRE(GE(1).is_satisfied(1));
	REQUIRE(GE(1).is_satisfied(2));
	REQUIRE(!GT(1).is_satisfied(0));
	REQUIRE(!GT(1).is_satisfied(1));
	REQUIRE(GT(1).is_satisfied(2));
}

TEST_CASE("Simple TA", "[ta]")
{
	TimedAutomaton<std::string> ta{"s0", {"s0"}};
	ta.add_transition(Transition<std::string>("s0", "a", "s0"));
	REQUIRE(ta.accepts_word({}));
	REQUIRE(ta.accepts_word({{"a", 0}}));
	REQUIRE(ta.accepts_word({{"a", 1}}));
	REQUIRE(ta.accepts_word({{"a", 1}, {"a", 1}, {"a", 1}, {"a", 1}}));
	REQUIRE(!ta.accepts_word({{"b", 0}}));
	REQUIRE(!ta.accepts_word({{"a", 1}, {"a", 0}}));
}

TEST_CASE("TA with a simple guard", "[ta]")
{
	TimedAutomaton<std::string> ta{"s0", {"s0"}};
	ClockConstraint             c = AtomicClockConstraintT<std::less<Time>>(1);
	ta.add_clock("x");
	ta.add_transition(Transition<std::string>("s0", "a", "s0", {{"x", c}}));
	REQUIRE(!ta.accepts_word({{"a", 2}}));
	REQUIRE(ta.accepts_word({{"a", 0.5}}));
	REQUIRE(!ta.accepts_word({{"a", 1}}));
}

TEST_CASE("TA with clock reset", "[ta]")
{
	TimedAutomaton<std::string> ta{"s0", {"s0"}};
	ClockConstraint             c = AtomicClockConstraintT<std::less<Time>>(2);
	ta.add_clock("x");
	ta.add_transition(Transition<std::string>("s0", "a", "s0", {{"x", c}}, {"x"}));
	REQUIRE(ta.accepts_word({{"a", 1}, {"a", 2}, {"a", 3}}));
	REQUIRE(!ta.accepts_word({{"a", 1}, {"a", 3}, {"a", 3}}));
}

TEST_CASE("Simple non-deterministic TA", "[ta]")
{
	TimedAutomaton<std::string> ta{"s0", {"s2"}};
	ta.add_location("s1");
	ta.add_transition(Transition<std::string>("s0", "a", "s1"));
	ta.add_transition(Transition<std::string>("s0", "a", "s2"));
	ta.add_transition(Transition<std::string>("s1", "b", "s1"));
	ta.add_transition(Transition<std::string>("s2", "b", "s2"));
	REQUIRE(ta.accepts_word({{"a", 1}, {"b", 2}}));
}

TEST_CASE("Non-determinstic TA with clocks", "[ta]")
{
	TimedAutomaton<std::string> ta{"s0", {"s1", "s2"}};
	ta.add_location("s1");
	ta.add_clock("x");
	ClockConstraint c1 = AtomicClockConstraintT<std::less<Time>>(2);
	ClockConstraint c2 = AtomicClockConstraintT<std::greater<Time>>(2);
	ta.add_transition(Transition<std::string>("s0", "a", "s1"));
	ta.add_transition(Transition<std::string>("s0", "a", "s2"));
	ta.add_transition(Transition<std::string>("s1", "b", "s1", {{"x", c1}}));

	REQUIRE(ta.accepts_word({{"a", 1}, {"b", 1}}));
	REQUIRE(!ta.accepts_word({{"a", 1}, {"b", 3}}));

	ta.add_transition(Transition<std::string>("s2", "b", "s2", {{"x", c2}}));

	REQUIRE(ta.accepts_word({{"a", 1}, {"b", 1}}));
	REQUIRE(ta.accepts_word({{"a", 1}, {"b", 3}}));
}

TEST_CASE("Transitions must use the TA's locations and clocks", "[ta]")
{
	TimedAutomaton<std::string> ta{"s0", {"s0"}};
	ta.add_location("s1");
	ta.add_clock("x");

	ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(2);
	REQUIRE_THROWS_AS(ta.add_transition(Transition<std::string>("s0", "a", "s2")),
	                  InvalidLocationException<std::string>);
	REQUIRE_THROWS_AS(ta.add_transition(Transition<std::string>("s2", "a", "s0")),
	                  InvalidLocationException<std::string>);
	REQUIRE_THROWS_AS(ta.add_transition(Transition<std::string>("s0", "a", "s1", {{"y", c}})),
	                  InvalidClockException);
	REQUIRE_THROWS_AS(ta.add_transition(Transition<std::string>("s0", "a", "s1", {}, {"y"})),
	                  InvalidClockException);
}

TEST_CASE("Create a TA with non-string location types", "[ta]")
{
	TimedAutomaton<unsigned int> ta{0, {0}};
	ClockConstraint              c = AtomicClockConstraintT<std::less<Time>>(1);
	ta.add_clock("x");
	ta.add_transition(Transition<unsigned int>(0, "a", 0, {{"x", c}}));
	REQUIRE(!ta.accepts_word({{"a", 2}}));
	REQUIRE(ta.accepts_word({{"a", 0.5}}));
	REQUIRE(!ta.accepts_word({{"a", 1}}));
}
