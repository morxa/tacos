/***************************************************************************
 *  test_number_utilities.cpp - Test utility-functions for numbers
 *
 *  Created: 7 December 2020
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

#include <utilities/numbers.h>

#include <catch2/catch.hpp>

namespace {

TEST_CASE("Get fractional and integer parts of numbers", "[libutilities]")
{
	using namespace utilities;
	auto frac1 = getFractionalPart<int, double>(2.4);
	REQUIRE_THAT(frac1,
	             Catch::Matchers::WithinRel(0.4, 0.0000001) && Catch::Matchers::WithinULP(0.4, 8));
	auto frac2 = getFractionalPart<int, double>(2.0);
	REQUIRE_THAT(frac2,
	             Catch::Matchers::WithinRel(0.0, 0.0000001) && Catch::Matchers::WithinULP(0.0, 8));

	REQUIRE(getIntegerPart<int, double>(2.4) == 2);
	REQUIRE(getIntegerPart<int, double>(2.0) == 2);

	REQUIRE(!isInteger<int, double>(2.4));
	REQUIRE(isInteger<int, double>(2.0));
}

} // namespace
