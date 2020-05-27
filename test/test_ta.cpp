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

#include <libta/ta.h>

#include <catch2/catch.hpp>
#include <functional>

using namespace ta;

TEST_CASE("Clock constraints with integers", "[libta]")
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

TEST_CASE("Clock constraints with doubles", "[libta]")
{
	using LT = AtomicClockConstraintT<std::less<Time>>;
	using LE = AtomicClockConstraintT<std::less_equal<Time>>;
	using EQ = AtomicClockConstraintT<std::equal_to<Time>>;
	using GE = AtomicClockConstraintT<std::greater_equal<Time>>;
	using GT = AtomicClockConstraintT<std::greater<Time>>;
	REQUIRE(LT(0.1).is_satisfied(0.0));
	REQUIRE(!LT(0.1).is_satisfied(0.1));
	REQUIRE(!LT(0.1).is_satisfied(0.2));
	REQUIRE(LE(0.1).is_satisfied(0.0));
	REQUIRE(LE(0.1).is_satisfied(0.1));
	REQUIRE(!LE(0.1).is_satisfied(0.2));
	REQUIRE(!EQ(0.1).is_satisfied(0.0));
	REQUIRE(EQ(0.1).is_satisfied(0.1));
	REQUIRE(!EQ(0.1).is_satisfied(0.2));
	REQUIRE(!GE(0.1).is_satisfied(0.0));
	REQUIRE(GE(0.1).is_satisfied(0.1));
	REQUIRE(GE(0.1).is_satisfied(0.2));
	REQUIRE(!GT(0.1).is_satisfied(0.0));
	REQUIRE(!GT(0.1).is_satisfied(0.1));
	REQUIRE(GT(0.1).is_satisfied(0.2));
}

TEST_CASE("Simple TA", "[libta]")
{
	TimedAutomaton ta{"s0", {"s0"}};
	ta.add_transition(Transition("s0", "a", "s0"));
	SECTION("accepting the empty word")
	{
		REQUIRE(ta.accepts_word({}));
	}
	SECTION("accepting a single 'a'")
	{
		REQUIRE(ta.accepts_word({{"a", 0}}));
	}
	SECTION("accepting a single 'a' at time 1")
	{
		REQUIRE(ta.accepts_word({{"a", 1}}));
	}
	SECTION("accepting multiple 'a's")
	{
		REQUIRE(ta.accepts_word({{"a", 1}, {"a", 1}, {"a", 1}, {"a", 1}}));
	}
	SECTION("not accepting 'b'")
	{
		REQUIRE(!ta.accepts_word({{"b", 0}}));
	}
	SECTION("not accepting an ill-formed word")
	{
		REQUIRE(!ta.accepts_word({{"a", 1}, {"a", 0}}));
	}
}

TEST_CASE("TA with a simple guard", "[libta]")
{
	TimedAutomaton  ta{"s0", {"s0"}};
	ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(1);
	ta.add_clock("x");
	ta.add_transition(Transition("s0", "a", "s0", {{"x", c}}));
	SECTION("not accepting a single 'a' at time 2")
	{
		REQUIRE(!ta.accepts_word({{"a", 2}}));
	}
	SECTION("accepting a single 'a' at time 0.5")
	{
		REQUIRE(ta.accepts_word({{"a", 0.5}}));
	}
	SECTION("not accepting a single 'a' at time 1")
	{
		REQUIRE(!ta.accepts_word({{"a", 1}}));
	}
}
