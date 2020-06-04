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
}
