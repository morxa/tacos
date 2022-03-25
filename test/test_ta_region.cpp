/***************************************************************************
 *  test_ta_region.cpp - Test TA-Region implementation
 *
 *  Created: Mon 14 December 2020
 *  Copyright  2020  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



#include "automata/automata.h"
#include "automata/ta_regions.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

namespace {

using namespace tacos;
using namespace automata;
using namespace automata::ta;
using Location = Location<std::string>;

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

TEST_CASE("Get clock constraint from region", "[taRegion]")
{
	// both
	CHECK(get_clock_constraints_from_region_index(0, 5)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::equal_to<Time>>{0}});
	CHECK(get_clock_constraints_from_region_index(1, 5)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::greater<Time>>{0},
	                                      AtomicClockConstraintT<std::less<Time>>{1}});
	CHECK(get_clock_constraints_from_region_index(2, 5)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::equal_to<Time>>{1}});
	CHECK(get_clock_constraints_from_region_index(3, 5)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::greater<Time>>{1},
	                                      AtomicClockConstraintT<std::less<Time>>{2}});
	CHECK(get_clock_constraints_from_region_index(4, 5)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::equal_to<Time>>{2}});
	CHECK(get_clock_constraints_from_region_index(5, 5)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::greater<Time>>{2}});

	// lower
	CHECK(get_clock_constraints_from_region_index(0, 5, ConstraintBoundType::LOWER)
	      == std::vector<ClockConstraint>{});
	CHECK(get_clock_constraints_from_region_index(1, 5, ConstraintBoundType::LOWER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::greater<Time>>{0}});
	CHECK(get_clock_constraints_from_region_index(2, 5, ConstraintBoundType::LOWER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::greater_equal<Time>>{1}});
	CHECK(get_clock_constraints_from_region_index(3, 5, ConstraintBoundType::LOWER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::greater<Time>>{1}});
	CHECK(get_clock_constraints_from_region_index(4, 5, ConstraintBoundType::LOWER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::greater_equal<Time>>{2}});
	CHECK(get_clock_constraints_from_region_index(5, 5, ConstraintBoundType::LOWER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::greater<Time>>{2}});

	// upper
	CHECK(get_clock_constraints_from_region_index(0, 5, ConstraintBoundType::UPPER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::equal_to<Time>>{0}});
	CHECK(get_clock_constraints_from_region_index(1, 5, ConstraintBoundType::UPPER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::less<Time>>{1}});
	CHECK(get_clock_constraints_from_region_index(2, 5, ConstraintBoundType::UPPER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::less_equal<Time>>{1}});
	CHECK(get_clock_constraints_from_region_index(3, 5, ConstraintBoundType::UPPER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::less<Time>>{2}});
	CHECK(get_clock_constraints_from_region_index(4, 5, ConstraintBoundType::UPPER)
	      == std::vector<ClockConstraint>{AtomicClockConstraintT<std::less_equal<Time>>{2}});
	CHECK(get_clock_constraints_from_region_index(5, 5, ConstraintBoundType::UPPER)
	      == std::vector<ClockConstraint>{});
}

} // namespace
