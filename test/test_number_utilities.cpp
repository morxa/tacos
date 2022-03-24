/***************************************************************************
 *  test_number_utilities.cpp - Test utility-functions for numbers
 *
 *  Created: 7 December 2020
 *  Copyright  2020  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



#include "utilities/numbers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <limits>

namespace {

using namespace tacos;

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

TEST_CASE("Approximate float comparison", "[libutilities]")
{
	utilities::ApproxFloatComparator<float> comp;
	CHECK(!comp(1.0, 1.0));
	CHECK(!comp(1.0 - std::numeric_limits<float>::epsilon(), 1.0));
	CHECK(!comp(1.0 - 4 * std::numeric_limits<float>::epsilon(), 1.0));
	CHECK(comp(1.0 - 10 * std::numeric_limits<float>::epsilon(), 1.0));
	CHECK(!comp(1.0, 1.0 - 10 * std::numeric_limits<float>::epsilon()));
	CHECK(comp(1.0, 1.0 + 10 * std::numeric_limits<float>::epsilon()));
	CHECK(comp(0.5, 1.0));
	CHECK(!comp(1.5, 1.0));
}

} // namespace
