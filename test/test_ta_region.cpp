/***************************************************************************
 *  test_ta_region.cpp - Test TA-Region implementation
 *
 *  Created: Mon 14 December 2020
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

#include <automata/ta_regions.h>

#include <catch2/catch.hpp>

namespace {

using namespace automata;
using namespace automata::ta;

TEST_CASE("Region index", "[taRegion]")
{
	TimedAutomatonRegions regionSet{4};
	CHECK(regionSet.getRegionIndex(4.1) == std::size_t(9));
	CHECK(regionSet.getRegionIndex(4.0) == std::size_t(8));
	CHECK(regionSet.getRegionIndex(3.9) == std::size_t(7));
	CHECK(regionSet.getRegionIndex(0.0) == std::size_t(0));
	CHECK(regionSet.getRegionIndex(0.1) == std::size_t(1));
}

} // namespace
