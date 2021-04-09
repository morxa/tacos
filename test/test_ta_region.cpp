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

#include "automata/ta_regions.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

namespace {

using namespace automata;
using namespace automata::ta;
using Location = automata::ta::Location<std::string>;

TEST_CASE("Region index", "[taRegion]")
{
	TimedAutomatonRegions regionSet{4};
	CHECK(regionSet.getRegionIndex(4.1) == std::size_t(9));
	CHECK(regionSet.getRegionIndex(4.0) == std::size_t(8));
	CHECK(regionSet.getRegionIndex(3.9) == std::size_t(7));
	CHECK(regionSet.getRegionIndex(0.0) == std::size_t(0));
	CHECK(regionSet.getRegionIndex(0.1) == std::size_t(1));
}

TEST_CASE("Region Candidate", "[taRegion]")
{
	const auto candidate =
	  get_region_candidate<std::string>({Location{"s0"}, {{"c0", 2}, {"c1", 3}, {"c2", 0}}});
	CHECK(candidate.location == Location{"s0"});
	const auto &clock_set_valuation = candidate.clock_valuations;
	REQUIRE(clock_set_valuation.find("c0") != clock_set_valuation.end());
	CHECK_THAT(clock_set_valuation.at("c0"), Catch::Matchers::WithinULP(1.0, 4));
	REQUIRE(clock_set_valuation.find("c1") != clock_set_valuation.end());
	CHECK(clock_set_valuation.at("c1") > 1.0);
	CHECK(clock_set_valuation.at("c1") < 2.0);
	REQUIRE(clock_set_valuation.find("c2") != clock_set_valuation.end());
	CHECK_THAT(clock_set_valuation.at("c2"), Catch::Matchers::WithinULP(0.0, 4));
}

TEST_CASE("Get largest region index", "[taRegion]")
{
	TimedAutomaton<std::string, std::string> ta{{"a", "b"},
	                                            Location{"s0"},
	                                            {Location{"s1"}, Location{"s2"}}};
	ta.add_location(Location{"s1"});
	ta.add_location(Location{"s2"});
	ta.add_clock("x");
	SECTION("max constant is 3")
	{
		ClockConstraint c1 = AtomicClockConstraintT<std::less<Time>>(2);
		ClockConstraint c2 = AtomicClockConstraintT<std::greater<Time>>(3);
		ta.add_transition(Transition<std::string, std::string>(Location{"s0"}, "a", Location{"s1"}));
		ta.add_transition(
		  Transition<std::string, std::string>(Location{"s0"}, "a", Location{"s2"}, {{"x", c2}}));
		ta.add_transition(
		  Transition<std::string, std::string>(Location{"s1"}, "b", Location{"s1"}, {{"x", c1}}));
		CHECK(get_maximal_region_index(ta) == RegionIndex(7));
	}
	SECTION("max constant is 2")
	{
		ClockConstraint c1 = AtomicClockConstraintT<std::less<Time>>(1);
		ClockConstraint c2 = AtomicClockConstraintT<std::greater<Time>>(2);
		ta.add_transition(Transition<std::string, std::string>(Location{"s0"}, "a", Location{"s1"}));
		ta.add_transition(
		  Transition<std::string, std::string>(Location{"s0"}, "a", Location{"s2"}, {{"x", c2}}));
		ta.add_transition(
		  Transition<std::string, std::string>(Location{"s1"}, "b", Location{"s1"}, {{"x", c1}}));
		CHECK(get_maximal_region_index(ta) == RegionIndex(5));
	}
	SECTION("max constant is 1")
	{
		ClockConstraint c1 = AtomicClockConstraintT<std::less<Time>>(1);
		ClockConstraint c2 = AtomicClockConstraintT<std::greater<Time>>(1);
		ta.add_transition(Transition<std::string, std::string>(Location{"s0"}, "a", Location{"s1"}));
		ta.add_transition(
		  Transition<std::string, std::string>(Location{"s0"}, "a", Location{"s2"}, {{"x", c2}}));
		ta.add_transition(
		  Transition<std::string, std::string>(Location{"s1"}, "b", Location{"s1"}, {{"x", c1}}));
		CHECK(get_maximal_region_index(ta) == RegionIndex(3));
	}
}

} // namespace
