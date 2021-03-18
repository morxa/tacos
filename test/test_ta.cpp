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

#include "automata/automata.h"
#include "automata/ta.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <functional>

namespace {

using namespace automata;
using namespace automata::ta;
using Configuration = automata::ta::Configuration<std::string>;
using Catch::Matchers::Contains;
using TimedAutomaton = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition     = automata::ta::Transition<std::string, std::string>;
using Location       = Location<std::string>;

TEST_CASE("Clock constraints with integers", "[ta]")
{
	using LT = AtomicClockConstraintT<std::less<Time>>;
	using LE = AtomicClockConstraintT<std::less_equal<Time>>;
	using EQ = AtomicClockConstraintT<std::equal_to<Time>>;
	using GE = AtomicClockConstraintT<std::greater_equal<Time>>;
	using GT = AtomicClockConstraintT<std::greater<Time>>;
	CHECK(LT(1).is_satisfied(0));
	CHECK(!LT(1).is_satisfied(1));
	CHECK(!LT(1).is_satisfied(2));
	CHECK(LE(1).is_satisfied(0));
	CHECK(LE(1).is_satisfied(1));
	CHECK(!LE(1).is_satisfied(2));
	CHECK(!EQ(1).is_satisfied(0));
	CHECK(EQ(1).is_satisfied(1));
	CHECK(!EQ(1).is_satisfied(2));
	CHECK(!GE(1).is_satisfied(0));
	CHECK(GE(1).is_satisfied(1));
	CHECK(GE(1).is_satisfied(2));
	CHECK(!GT(1).is_satisfied(0));
	CHECK(!GT(1).is_satisfied(1));
	CHECK(GT(1).is_satisfied(2));
}

TEST_CASE("Simple TA", "[ta]")
{
	TimedAutomaton ta{{"a", "b"}, Location{"s0"}, {Location{"s0"}}};
	ta.add_transition(Transition(Location{"s0"}, "a", Location{"s0"}));

	CHECK(ta.get_initial_configuration() == Configuration{Location{"s0"}, {}});

	CHECK(ta.make_symbol_step({{Location{"s0"}}, {}}, "a")
	      == std::set{Configuration{Location{"s0"}, {}}});
	CHECK(ta.make_symbol_step({{Location{"s0"}}, {}}, "b").empty());

	CHECK(ta.accepts_word({}));
	CHECK(ta.accepts_word({{"a", 0}}));
	CHECK(ta.accepts_word({{"a", 1}}));
	CHECK(ta.accepts_word({{"a", 1}, {"a", 1}, {"a", 1}, {"a", 1}}));
	CHECK(!ta.accepts_word({{"b", 0}}));
	CHECK(!ta.accepts_word({{"a", 1}, {"a", 0}}));
}

TEST_CASE("Simple TA with two locations", "[ta]")
{
	TimedAutomaton ta{{"a", "b"}, Location{"s0"}, {Location{"s1"}}};
	ta.add_transition(Transition(Location{"s0"}, "a", Location{"s0"}));
	ta.add_transition(Transition(Location{"s0"}, "b", Location{"s1"}));
	// We must be in a final location.
	CHECK(!ta.accepts_word({{"a", 0}}));
	CHECK(ta.accepts_word({{"b", 0}}));
}

TEST_CASE("TA with a simple guard", "[ta]")
{
	TimedAutomaton  ta{{"a"}, Location{"s0"}, {Location{"s0"}}};
	ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(1);
	ta.add_clock("x");
	ta.add_transition(Transition(Location{"s0"}, "a", Location{"s0"}, {{"x", c}}));

	CHECK(ta.get_initial_configuration() == Configuration{Location{"s0"}, {{"x", 0}}});

	CHECK(ta.make_symbol_step({Location{"s0"}, {{"x", 0}}}, "a")
	      == std::set{Configuration{Location{"s0"}, {{"x", 0}}}});
	CHECK(ta.make_symbol_step({Location{"s0"}, {{"x", 1}}}, "a").empty());

	CHECK(!ta.accepts_word({{"a", 2}}));
	CHECK(ta.accepts_word({{"a", 0.5}}));
	CHECK(!ta.accepts_word({{"a", 1}}));
}

TEST_CASE("TA with clock reset", "[ta]")
{
	SECTION("Build TA step by step")
	{
		TimedAutomaton  ta{{"a"}, Location{"s0"}, {Location{"s0"}}};
		ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(2);
		ta.add_clock("x");
		ta.add_transition(Transition(Location{"s0"}, "a", Location{"s0"}, {{"x", c}}, {"x"}));
		CHECK(ta.get_initial_configuration() == Configuration{Location{"s0"}, {{"x", 0}}});

		CHECK(ta.make_symbol_step({Location{"s0"}, {{"x", 1}}}, "a")
		      == std::set{Configuration{Location{"s0"}, {{"x", 0}}}});

		CHECK(ta.accepts_word({{"a", 1}, {"a", 2}, {"a", 3}}));
		CHECK(!ta.accepts_word({{"a", 1}, {"a", 3}, {"a", 3}}));
	}
	SECTION("Build TA with a single constructor call")
	{
		TimedAutomaton ta{{Location{"s0"}},
		                  {"a"},
		                  Location{"s0"},
		                  {Location{"s0"}},
		                  {"x"},
		                  {Transition{Location{"s0"},
		                              "a",
		                              Location{"s0"},
		                              {{"x", AtomicClockConstraintT<std::less<Time>>(2)}},
		                              {"x"}}}};
		CHECK(ta.get_initial_configuration() == Configuration{Location{"s0"}, {{"x", 0}}});

		CHECK(ta.make_symbol_step({Location{"s0"}, {{"x", 1}}}, "a")
		      == std::set{Configuration{Location{"s0"}, {{"x", 0}}}});

		CHECK(ta.accepts_word({{"a", 1}, {"a", 2}, {"a", 3}}));
		CHECK(!ta.accepts_word({{"a", 1}, {"a", 3}, {"a", 3}}));
	}
}

TEST_CASE("Simple non-deterministic TA", "[ta]")
{
	TimedAutomaton ta{{"a", "b"}, Location{"s0"}, {Location{"s2"}}};
	ta.add_location(Location{"s1"});
	ta.add_transition(Transition(Location{"s0"}, "a", Location{"s1"}));
	ta.add_transition(Transition(Location{"s0"}, "a", Location{"s2"}));
	ta.add_transition(Transition(Location{"s1"}, "b", Location{"s1"}));
	ta.add_transition(Transition(Location{"s2"}, "b", Location{"s2"}));

	CHECK(ta.make_symbol_step({Location{"s0"}, {}}, "a")
	      == std::set{Configuration{Location{"s1"}, {}}, Configuration{Location{"s2"}, {}}});

	CHECK(ta.accepts_word({{"a", 1}, {"b", 2}}));
}

TEST_CASE("Non-determinstic TA with clocks", "[ta]")
{
	TimedAutomaton ta{{"a", "b"}, Location{"s0"}, {Location{"s1"}, Location{"s2"}}};
	ta.add_location(Location{"s1"});
	ta.add_clock("x");
	ClockConstraint c1 = AtomicClockConstraintT<std::less<Time>>(2);
	ta.add_transition(Transition(Location{"s0"}, "a", Location{"s1"}));
	ta.add_transition(Transition(Location{"s0"}, "a", Location{"s2"}));
	ta.add_transition(Transition(Location{"s1"}, "b", Location{"s1"}, {{"x", c1}}));

	CHECK(ta.accepts_word({{"a", 1}, {"b", 1}}));
	CHECK(!ta.accepts_word({{"a", 1}, {"b", 3}}));

	ClockConstraint c2 = AtomicClockConstraintT<std::greater<Time>>(2);
	ta.add_transition(Transition(Location{"s2"}, "b", Location{"s2"}, {{"x", c2}}));

	CHECK(ta.accepts_word({{"a", 1}, {"b", 1}}));
	CHECK(ta.accepts_word({{"a", 1}, {"b", 3}}));
}

TEST_CASE("Transitions must use the TA's alphabet, locations and clocks", "[ta]")
{
	TimedAutomaton ta{{"a", "b"}, Location{"s0"}, {Location{"s0"}}};
	ta.add_location(Location{"s1"});
	ta.add_clock("x");

	ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(2);
	CHECK_THROWS_AS(ta.add_transition(Transition(Location{"s0"}, "a", Location{"s2"})),
	                InvalidLocationException<std::string>);
	CHECK_THROWS_AS(ta.add_transition(Transition(Location{"s2"}, "a", Location{"s0"})),
	                InvalidLocationException<std::string>);
	CHECK_THROWS_AS(ta.add_transition(Transition(Location{"s0"}, "a", Location{"s1"}, {{"y", c}})),
	                InvalidClockException);
	CHECK_THROWS_AS(ta.add_transition(Transition(Location{"s0"}, "a", Location{"s1"}, {}, {"y"})),
	                InvalidClockException);
	CHECK_THROWS_AS(ta.add_transition(Transition(Location{"s0"}, "c", Location{"s0"})),
	                InvalidSymbolException);
}

TEST_CASE("Create a TA with non-string location types", "[ta]")
{
	using Location = automata::ta::Location<unsigned int>;
	automata::ta::TimedAutomaton<unsigned int, std::string> ta{{"a"}, Location{0}, {Location{0}}};
	ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(1);
	ta.add_clock("x");
	ta.add_transition(
	  automata::ta::Transition<unsigned int, std::string>(Location{0}, "a", Location{0}, {{"x", c}}));
	CHECK(!ta.accepts_word({{"a", 2}}));
	CHECK(ta.accepts_word({{"a", 0.5}}));
	CHECK(!ta.accepts_word({{"a", 1}}));
}

TEST_CASE("Get enabled transitions", "[ta]")
{
	TimedAutomaton ta{{"a", "b"}, Location{"s0"}, {Location{"s1"}}};
	Transition     t1{Location{"s0"}, "a", Location{"s1"}};
	ta.add_transition(t1);
	CHECK(ta.get_enabled_transitions({Location{"s0"}, {}}) == std::vector<Transition>{{t1}});
	Transition t2{Location{"s1"}, "a", Location{"s1"}};
	ta.add_transition(t2);
	// t2 should not be enabled
	CHECK(ta.get_enabled_transitions({Location{"s0"}, {}}) == std::vector<Transition>{{t1}});
	Transition t3{Location{"s0"}, "b", Location{"s0"}};
	ta.add_transition(t3);
	// t3 should be enabled
	CHECK(ta.get_enabled_transitions({Location{"s0"}, {{"c0", 0}}})
	      == std::vector<Transition>{{t1, t3}});
	ta.add_clock("c0");
	Transition t4{Location{"s0"},
	              "b",
	              Location{"s0"},
	              {{"c0", AtomicClockConstraintT<std::greater<Time>>(1)}}};
	// t4 should not be enabled
	CHECK(ta.get_enabled_transitions({Location{"s0"}, {}}) == std::vector<Transition>{{t1, t3}});
	Transition t5{Location{"s0"},
	              "b",
	              Location{"s0"},
	              {{"c0", AtomicClockConstraintT<std::less<Time>>(1)}}};
	ta.add_transition(t5);
	// t5 should be enabled
	CHECK(ta.get_enabled_transitions({Location{"s0"}, {{"c0", 0}}})
	      == std::vector<Transition>{{t1, t3, t5}});
}

TEST_CASE("Constructing invalid TAs throws exceptions", "[ta]")
{
	CHECK_THROWS(
	  TimedAutomaton({Location{"l0"}}, {"a"}, Location{"non_existent_initial_location"}, {}, {}, {}));
	CHECK_THROWS(TimedAutomaton(
	  {Location{"l0"}}, {"a"}, Location{"l0"}, {Location{"non_existent_final_location"}}, {}, {}));
	CHECK_THROWS(TimedAutomaton({Location{"l0"}},
	                            {"a"},
	                            Location{"l0"},
	                            {Location{"l0"}},
	                            {"x"},
	                            {Transition{Location{"s0"},
	                                        "a",
	                                        Location{"s0"},
	                                        {{"y", AtomicClockConstraintT<std::less<Time>>(2)}},
	                                        {"x"}}}));
}

// TODO Test case with multiple clocks

} // namespace
