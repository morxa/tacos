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

#include <libmtl/Interval.h>

#include <catch2/catch.hpp>

TEST_CASE("Construction of intervals", "[libmtl]")
{
	using Interval = arithmetic::Interval<int>;

	REQUIRE(Interval(2, 3).lower() == 2);
	REQUIRE(Interval(2, 3).upper() == 3);
	REQUIRE(Interval().lowerBoundType() == arithmetic::BoundType::INFTY);
	REQUIRE(Interval().upperBoundType() == arithmetic::BoundType::INFTY);
}

TEST_CASE("Emptiness", "[libmtl]")
{
	using namespace arithmetic;
	using Interval = Interval<int>;

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
	using namespace arithmetic;
	using Interval = Interval<int>;

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
