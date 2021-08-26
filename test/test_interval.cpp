/***************************************************************************
 *  test_interval.cpp - Test Interval implementation
 *
 *  Created: 4 June 2020
 *  Copyright  2020  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
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

#include "utilities/Interval.h"

#include <catch2/catch_test_macros.hpp>
#include <sstream>

namespace {
using namespace tacos;

using Interval = utilities::arithmetic::Interval<int>;
using utilities::arithmetic::BoundType;

TEST_CASE("Construction of intervals", "[libmtl]")
{
	REQUIRE(Interval(2, 3).lower() == 2);
	REQUIRE(Interval(2, 3).upper() == 3);
	REQUIRE(Interval().lowerBoundType() == BoundType::INFTY);
	REQUIRE(Interval().upperBoundType() == BoundType::INFTY);
}

TEST_CASE("Interval comparison", "[libmtl]")
{
	// Less than
	CHECK(!(Interval() < Interval()));
	CHECK(Interval(0, BoundType::INFTY, 2, BoundType::WEAK) < Interval());
	CHECK(!(Interval(1, BoundType::WEAK, 2, BoundType::WEAK) < Interval()));
	CHECK(!(Interval(1, BoundType::WEAK, 2, BoundType::WEAK)
	        < Interval(0, BoundType::INFTY, 1, BoundType::WEAK)));
	CHECK(!(Interval(1, BoundType::WEAK, 2, BoundType::WEAK)
	        < Interval(1, BoundType::WEAK, 2, BoundType::WEAK)));
	CHECK(Interval(1, BoundType::WEAK, 2, BoundType::WEAK)
	      < Interval(2, BoundType::WEAK, 3, BoundType::WEAK));
	CHECK(Interval(1, BoundType::WEAK, 2, BoundType::WEAK)
	      < Interval(3, BoundType::WEAK, 4, BoundType::WEAK));
	CHECK(Interval(1, BoundType::WEAK, 2, BoundType::WEAK)
	      < Interval(2, BoundType::STRICT, 3, BoundType::WEAK));
	CHECK(Interval(1, BoundType::WEAK, 2, BoundType::STRICT)
	      < Interval(2, BoundType::WEAK, 3, BoundType::WEAK));

	// Bigger than
	CHECK(!(Interval() < Interval()));
	CHECK(Interval() > Interval(0, BoundType::INFTY, 2, BoundType::WEAK));
	CHECK(!(Interval() > Interval(1, BoundType::WEAK, 2, BoundType::WEAK)));
	CHECK(!(Interval(0, BoundType::INFTY, 1, BoundType::WEAK)
	        > Interval(1, BoundType::WEAK, 2, BoundType::WEAK)));
	CHECK(!(Interval(1, BoundType::WEAK, 2, BoundType::WEAK)
	        > Interval(1, BoundType::WEAK, 2, BoundType::WEAK)));
	CHECK(Interval(2, BoundType::WEAK, 3, BoundType::WEAK)
	      > Interval(1, BoundType::WEAK, 2, BoundType::WEAK));
	CHECK(Interval(3, BoundType::WEAK, 4, BoundType::WEAK)
	      > Interval(1, BoundType::WEAK, 2, BoundType::WEAK));
	CHECK(Interval(2, BoundType::STRICT, 3, BoundType::WEAK)
	      > Interval(1, BoundType::WEAK, 2, BoundType::WEAK));
	CHECK(Interval(2, BoundType::WEAK, 3, BoundType::WEAK)
	      > Interval(1, BoundType::WEAK, 2, BoundType::STRICT));

	// Equality
	CHECK(Interval() == Interval());
	CHECK(Interval(1, BoundType::WEAK, 0, BoundType::INFTY) != Interval());
	CHECK(Interval(1, BoundType::INFTY, 1, BoundType::INFTY) == Interval());
	CHECK(Interval() == Interval(1, BoundType::INFTY, 1, BoundType::INFTY));
	CHECK(Interval(1, BoundType::WEAK, 0, BoundType::INFTY)
	      != Interval(1, BoundType::STRICT, 0, BoundType::INFTY));
	CHECK(Interval(1, BoundType::WEAK, 2, BoundType::WEAK)
	      != Interval(1, BoundType::WEAK, 2, BoundType::STRICT));
	CHECK(Interval(1, BoundType::STRICT, 2, BoundType::STRICT)
	      != Interval(1, BoundType::WEAK, 2, BoundType::STRICT));
	CHECK(Interval(1, BoundType::STRICT, 2, BoundType::STRICT)
	      == Interval(1, BoundType::STRICT, 2, BoundType::STRICT));
	CHECK(Interval(2, BoundType::WEAK, 3, BoundType::STRICT)
	      == Interval(2, BoundType::WEAK, 3, BoundType::STRICT));
	CHECK(Interval(2, BoundType::WEAK, 4, BoundType::WEAK)
	      == Interval(2, BoundType::WEAK, 4, BoundType::WEAK));
}

TEST_CASE("Interval setters", "[libmtl]")
{
	Interval interval;
	SECTION("Lower weak bound")
	{
		interval.set_lower(2, BoundType::WEAK);
		CHECK(interval == Interval(2, BoundType::WEAK, 0, BoundType::INFTY));
	}
	SECTION("Lower strict bound")
	{
		interval.set_lower(2, BoundType::STRICT);
		CHECK(interval == Interval(2, BoundType::STRICT, 0, BoundType::INFTY));
	}
	SECTION("Upper weak bound")
	{
		interval.set_upper(3, BoundType::WEAK);
		CHECK(interval == Interval(0, BoundType::INFTY, 3, BoundType::WEAK));
	}
	SECTION("Upper weak bound")
	{
		interval.set_upper(3, BoundType::STRICT);
		CHECK(interval == Interval(0, BoundType::INFTY, 3, BoundType::STRICT));
	}
	SECTION("Infinity bounds")
	{
		Interval interval2(1, BoundType::WEAK, 2, BoundType::STRICT);
		interval2.set_lower(1, BoundType::INFTY);
		interval2.set_upper(2, BoundType::INFTY);
		// The bound values are ignored if the type is INFTY.
		CHECK(interval2 == Interval());
	}
}

TEST_CASE("Emptiness", "[libmtl]")
{
	REQUIRE(!Interval(2, 3).is_empty());
	REQUIRE(!Interval(3, 3).is_empty());
	REQUIRE(!Interval(2, BoundType::STRICT, 3, BoundType::WEAK).is_empty());
	REQUIRE(!Interval(2, BoundType::WEAK, 3, BoundType::STRICT).is_empty());
	REQUIRE(!Interval(2, BoundType::STRICT, 3, BoundType::STRICT).is_empty());
	REQUIRE(!Interval(2, BoundType::INFTY, 3, BoundType::WEAK).is_empty());
	REQUIRE(!Interval(2, BoundType::WEAK, 3, BoundType::INFTY).is_empty());
	REQUIRE(!Interval(2, BoundType::INFTY, 3, BoundType::STRICT).is_empty());
	REQUIRE(!Interval(2, BoundType::STRICT, 3, BoundType::INFTY).is_empty());
	REQUIRE(!Interval().is_empty());

	REQUIRE(Interval(3, 2).is_empty());
	REQUIRE(Interval(2, BoundType::STRICT, 2, BoundType::WEAK).is_empty());
	REQUIRE(Interval(2, BoundType::WEAK, 2, BoundType::STRICT).is_empty());
	REQUIRE(Interval(2, BoundType::STRICT, 2, BoundType::STRICT).is_empty());
}

TEST_CASE("Containment of values", "[libmtl]")
{
	REQUIRE(Interval(2, 3).contains(2));
	REQUIRE(Interval(2, 3).contains(3));
	REQUIRE(Interval(2, BoundType::WEAK, 3, BoundType::INFTY).contains(2));
	REQUIRE(Interval(2, BoundType::INFTY, 3, BoundType::WEAK).contains(3));
	REQUIRE(Interval().contains(2));
	REQUIRE(Interval(3, BoundType::INFTY, 2, BoundType::INFTY).contains(2));
	REQUIRE(Interval(3, BoundType::INFTY, 2, BoundType::INFTY).contains(4));

	REQUIRE(!Interval(2, BoundType::STRICT, 3, BoundType::INFTY).contains(2));
	REQUIRE(!Interval(2, BoundType::INFTY, 3, BoundType::STRICT).contains(3));
	REQUIRE(!Interval(2, BoundType::STRICT, 2, BoundType::STRICT).contains(2));
}

TEST_CASE("Print an interval", "[libmtl][print]")
{
	std::stringstream str;
	SECTION("No bounds")
	{
		str << Interval();
		CHECK(str.str() == "(∞, ∞)");
	}
	SECTION("weak lower bound")
	{
		str << Interval(1, BoundType::WEAK, 0, BoundType::INFTY);
		CHECK(str.str() == u8"(1, ∞)");
	}
	SECTION("strict lower bound")
	{
		str << Interval(2, BoundType::STRICT, 0, BoundType::INFTY);
		CHECK(str.str() == u8"[2, ∞)");
	}
	SECTION("weak upper bound")
	{
		str << Interval(0, BoundType::INFTY, 1, BoundType::WEAK);
		CHECK(str.str() == u8"(∞, 1)");
	}
	SECTION("strict upper bound")
	{
		str << Interval(0, BoundType::INFTY, 1, BoundType::STRICT);
		CHECK(str.str() == u8"(∞, 1]");
	}
	SECTION("weak lower and strict upper bound")
	{
		str << Interval(4, BoundType::WEAK, 5, BoundType::STRICT);
		CHECK(str.str() == "(4, 5]");
	}
}
} // namespace
