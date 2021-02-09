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

TEST_CASE("Region Candidate", "[taRegion]")
{
	const auto candidate =
	  get_region_candidate<std::string>({"s0", {{"c0", 2}, {"c1", 3}, {"c2", 0}}});
	CHECK(candidate.first == "s0");
	const auto &clock_set_valuation = candidate.second;
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
	TimedAutomaton<std::string, std::string> ta{{"a", "b"}, "s0", {"s1", "s2"}};
	ta.add_location("s1");
	ta.add_location("s2");
	ta.add_clock("x");
	SECTION("max constant is 3")
	{
		ClockConstraint c1 = AtomicClockConstraintT<std::less<Time>>(2);
		ClockConstraint c2 = AtomicClockConstraintT<std::greater<Time>>(3);
		ta.add_transition(Transition<std::string, std::string>("s0", "a", "s1"));
		ta.add_transition(Transition<std::string, std::string>("s0", "a", "s2", {{"x", c2}}));
		ta.add_transition(Transition<std::string, std::string>("s1", "b", "s1", {{"x", c1}}));
		CHECK(get_maximal_region_index(ta) == RegionIndex(7));
	}
	SECTION("max constant is 2")
	{
		ClockConstraint c1 = AtomicClockConstraintT<std::less<Time>>(1);
		ClockConstraint c2 = AtomicClockConstraintT<std::greater<Time>>(2);
		ta.add_transition(Transition<std::string, std::string>("s0", "a", "s1"));
		ta.add_transition(Transition<std::string, std::string>("s0", "a", "s2", {{"x", c2}}));
		ta.add_transition(Transition<std::string, std::string>("s1", "b", "s1", {{"x", c1}}));
		CHECK(get_maximal_region_index(ta) == RegionIndex(5));
	}
	SECTION("max constant is 1")
	{
		ClockConstraint c1 = AtomicClockConstraintT<std::less<Time>>(1);
		ClockConstraint c2 = AtomicClockConstraintT<std::greater<Time>>(1);
		ta.add_transition(Transition<std::string, std::string>("s0", "a", "s1"));
		ta.add_transition(Transition<std::string, std::string>("s0", "a", "s2", {{"x", c2}}));
		ta.add_transition(Transition<std::string, std::string>("s1", "b", "s1", {{"x", c1}}));
		CHECK(get_maximal_region_index(ta) == RegionIndex(3));
	}
}

} // namespace
