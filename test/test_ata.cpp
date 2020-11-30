/***************************************************************************
 *  test_ata.cpp - Tests for Alternating Timed Automata
 *
 *  Created: Tue 09 Jun 2020 09:05:03 CEST 09:05
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

#include <ta/ata.h>
#include <ta/ata_formula.h>
#include <ta/automata.h>

#include <catch2/catch.hpp>
#include <functional>
#include <memory>

namespace {

using namespace automata;
using namespace automata::ata;

TEST_CASE("Transitions in a single-state ATA", "[ta]")
{
	std::set<Transition<std::string, std::string>> transitions;
	transitions.insert(Transition<std::string, std::string>(
	  "s0", "a", std::make_unique<LocationFormula<std::string>>("s0")));
	AlternatingTimedAutomaton<std::string, std::string> ata({"a"},
	                                                        "s0",
	                                                        {"s0"},
	                                                        std::move(transitions));
	SECTION("reading a single 'a'")
	{
		auto runs = ata.make_symbol_transition({{}}, "a");
		REQUIRE(runs.size() == 1);
		REQUIRE(
		  runs[0]
		  == std::vector<std::pair<std::variant<Symbol, Time>, Configuration<std::string>>>{
		    {std::make_pair(std::variant<Symbol, Time>("a"), Configuration<std::string>{{"s0", 0}})}});
	}
	SECTION("reading 'a', '0', 'a'")
	{
		auto runs =
		  ata.make_symbol_transition(ata.make_time_transition(ata.make_symbol_transition({{}}, "a"), 1),
		                             "a");
		REQUIRE(runs.size() == 1);
		auto run = runs[0];
		REQUIRE(run.size() == 3);
		CHECK(
		  run[0]
		  == std::make_pair(std::variant<Symbol, Time>("a"), Configuration<std::string>{{"s0", 0}}));
		CHECK(run[1]
		      == std::make_pair(std::variant<Symbol, Time>(1.), Configuration<std::string>{{"s0", 1}}));
		CHECK(
		  run[2]
		  == std::make_pair(std::variant<Symbol, Time>("a"), Configuration<std::string>{{"s0", 1}}));
	}
}

TEST_CASE("ATA transition exceptions", "[ta]")
{
	std::set<Transition<std::string, std::string>> transitions;
	transitions.insert(Transition<std::string, std::string>(
	  "s0", "a", std::make_unique<LocationFormula<std::string>>("s0")));
	AlternatingTimedAutomaton<std::string, std::string> ata({"a"},
	                                                        "s0",
	                                                        {"s0"},
	                                                        std::move(transitions));
	SECTION("throwing if a time transition occurs before any symbol transition")
	{
		REQUIRE_THROWS(ata.make_time_transition({{}}, 0));
	}
	SECTION("throwing if two subsequent symbol transitions occur")
	{
		REQUIRE_THROWS(ata.make_symbol_transition(ata.make_symbol_transition({{}}, "a"), "a"));
	}
	SECTION("throwing if the time delta is negative")
	{
		auto runs = ata.make_symbol_transition({{}}, "a");
		REQUIRE_NOTHROW(ata.make_time_transition(runs, 0.));
		REQUIRE_NOTHROW(ata.make_time_transition(runs, 0.5));
		REQUIRE_THROWS_AS(ata.make_time_transition(runs, -0.5), NegativeTimeDeltaException);
	}
}

TEST_CASE("Simple ATA with a disjcuntion", "[ta]")
{
	std::set<Transition<std::string, std::string>> transitions;
	transitions.insert(
	  Transition<std::string, std::string>("s0",
	                                       "a",
	                                       std::make_unique<DisjunctionFormula<std::string>>(
	                                         std::make_unique<LocationFormula<std::string>>("s0"),
	                                         std::make_unique<LocationFormula<std::string>>("s1"))));
	AlternatingTimedAutomaton<std::string, std::string> ata({"a"},
	                                                        "s0",
	                                                        {"s0"},
	                                                        std::move(transitions));
	auto                                                runs = ata.make_symbol_transition({{}}, "a");
	REQUIRE(runs.size() == 2);
	auto run = runs[0];
	REQUIRE(run.size() == 1);
	CHECK(run[0].first == std::variant<Symbol, Time>("a"));
	CHECK(run[0].second == Configuration<std::string>{{"s0", 0}});
	run = runs[1];
	REQUIRE(run.size() == 1);
	CHECK(run[0].first == std::variant<Symbol, Time>("a"));
	CHECK(run[0].second == Configuration<std::string>{{"s1", 0}});
}

TEST_CASE("ATA accepting no events with a time difference of exactly 1"
          " (example by Ouaknine & Worrel, 2005)")
{
	std::set<Transition<std::string, std::string>> transitions;
	transitions.insert(Transition<std::string, std::string>(
	  "s0",
	  "a",
	  std::make_unique<ConjunctionFormula<std::string>>(
	    std::make_unique<LocationFormula<std::string>>("s0"),
	    std::make_unique<ResetClockFormula<std::string>>(
	      std::make_unique<LocationFormula<std::string>>("s1")))));
	transitions.insert(Transition<std::string, std::string>(
	  "s1",
	  "a",
	  std::make_unique<ConjunctionFormula<std::string>>(
	    std::make_unique<LocationFormula<std::string>>("s1"),
	    std::make_unique<ClockConstraintFormula<std::string>>(
	      AtomicClockConstraintT<std::not_equal_to<Time>>(1.)))));
	AlternatingTimedAutomaton<std::string, std::string> ata({"a"},
	                                                        "s0",
	                                                        {"s0", "s1"},
	                                                        std::move(transitions));

	auto runs = ata.make_symbol_transition({{}}, "a");
	REQUIRE(runs.size() == 1);
	auto run = runs[0];
	CHECK(run[0].first == std::variant<Symbol, Time>("a"));
	CHECK(run[0].second == Configuration<std::string>{{"s0", 0}, {"s1", 0}});
	runs = ata.make_symbol_transition(ata.make_time_transition(runs, 0.5), "a");
	REQUIRE(runs.size() == 1);
	run = runs[0];
	REQUIRE(run.size() == 3);
	CHECK(run[0].first == std::variant<Symbol, Time>("a"));
	CHECK(run[0].second == Configuration<std::string>{{"s0", 0}, {"s1", 0}});
	CHECK(run[1].first == std::variant<Symbol, Time>(0.5));
	CHECK(run[1].second == Configuration<std::string>{{"s0", 0.5}, {"s1", 0.5}});
	CHECK(run[2].first == std::variant<Symbol, Time>("a"));
	CHECK(run[2].second == Configuration<std::string>{{"s0", 0.5}, {"s1", 0}, {"s1", 0.5}});

	// (a,0), (a,1) should not be accepted
	runs = ata.make_symbol_transition({{}}, "a");
	runs = ata.make_time_transition(runs, 1);
	runs = ata.make_symbol_transition(runs, "a");
	CHECK(runs.size() == 0);

	SECTION("accepting the correct words")
	{
		CHECK(!ata.accepts_word({}));
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 1}}));
		CHECK(ata.accepts_word({{"a", 0}, {"a", 1.1}, {"a", 2}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 1.1}, {"a", 2}, {"a", 3}}));
	}
}

TEST_CASE("Time-bounded response two-state ATA (example by Ouaknine & Worrel, 2005)", "[ta]")
{
	std::set<Transition<std::string, std::string>> transitions;
	transitions.insert(Transition<std::string, std::string>(
	  "s0",
	  "a",
	  std::make_unique<ConjunctionFormula<std::string>>(
	    std::make_unique<LocationFormula<std::string>>("s0"),
	    std::make_unique<ResetClockFormula<std::string>>(
	      std::make_unique<LocationFormula<std::string>>("s1")))));
	transitions.insert(Transition<std::string, std::string>(
	  "s0", "b", std::make_unique<LocationFormula<std::string>>("s0")));
	transitions.insert(Transition<std::string, std::string>(
	  "s1", "a", std::make_unique<LocationFormula<std::string>>("s1")));
	transitions.insert(
	  Transition<std::string, std::string>("s1",
	                                       "b",
	                                       std::make_unique<DisjunctionFormula<std::string>>(
	                                         std::make_unique<ClockConstraintFormula<std::string>>(
	                                           AtomicClockConstraintT<std::equal_to<Time>>(1.)),
	                                         std::make_unique<LocationFormula<std::string>>("s1"))));
	AlternatingTimedAutomaton<std::string, std::string> ata({"a", "b"},
	                                                        "s0",
	                                                        {"s0"},
	                                                        std::move(transitions));

	auto runs = ata.make_symbol_transition({{}}, "a");
	runs      = ata.make_time_transition(runs, 1);
	runs      = ata.make_symbol_transition(runs, "b");
	REQUIRE(runs.size() == 2);
	auto run = runs[0];
	REQUIRE(run.size() == 3);
	CHECK(run[0].first == std::variant<Symbol, Time>("a"));
	CHECK(run[0].second == Configuration<std::string>{{"s0", 0}, {"s1", 0}});
	CHECK(run[1].first == std::variant<Symbol, Time>(1.0));
	CHECK(run[1].second == Configuration<std::string>{{"s0", 1.0}, {"s1", 1.0}});
	CHECK(run[2].first == std::variant<Symbol, Time>("b"));
	CHECK(run[2].second == Configuration<std::string>{{"s0", 1}});
	run = runs[1];
	REQUIRE(run.size() == 3);
	CHECK(run[0].first == std::variant<Symbol, Time>("a"));
	CHECK(run[0].second == Configuration<std::string>{{"s0", 0}, {"s1", 0}});
	CHECK(run[1].first == std::variant<Symbol, Time>(1.0));
	CHECK(run[1].second == Configuration<std::string>{{"s0", 1.0}, {"s1", 1.0}});
	CHECK(run[2].first == std::variant<Symbol, Time>("b"));
	CHECK(run[2].second == Configuration<std::string>{{"s0", 1}, {"s1", 1}});

	runs = ata.make_time_transition(runs, 0.5);
	runs = ata.make_symbol_transition(runs, "a");
	runs = ata.make_time_transition(runs, 1);
	runs = ata.make_symbol_transition(runs, "b");

	REQUIRE(runs.size() == 4);
	// 1. option: disjunct `x=1` on both b's
	run = runs[0];
	CHECK(run[3].first == std::variant<Symbol, Time>(0.5));
	CHECK(run[3].second == Configuration<std::string>{{"s0", 1.5}});
	CHECK(run[4].first == std::variant<Symbol, Time>("a"));
	CHECK(run[4].second == Configuration<std::string>{{"s0", 1.5}, {"s1", 0}});
	CHECK(run[5].first == std::variant<Symbol, Time>(1.));
	CHECK(run[5].second == Configuration<std::string>{{"s0", 2.5}, {"s1", 1}});
	CHECK(run[6].first == std::variant<Symbol, Time>("b"));
	CHECK(run[6].second == Configuration<std::string>{{"s0", 2.5}});

	// 2. option: disjunct `x=1` on first b, disjunct `s1` on second b
	run = runs[1];
	CHECK(run[3].first == std::variant<Symbol, Time>(0.5));
	CHECK(run[3].second == Configuration<std::string>{{"s0", 1.5}});
	CHECK(run[4].first == std::variant<Symbol, Time>("a"));
	CHECK(run[4].second == Configuration<std::string>{{"s0", 1.5}, {"s1", 0}});
	CHECK(run[5].first == std::variant<Symbol, Time>(1.));
	CHECK(run[5].second == Configuration<std::string>{{"s0", 2.5}, {"s1", 1}});
	CHECK(run[6].first == std::variant<Symbol, Time>("b"));
	CHECK(run[6].second == Configuration<std::string>{{"s0", 2.5}, {"s1", 1}});

	// 3. option: disjunct `s1` on first b, disjunct `x=1` on second b
	run = runs[2];
	CHECK(run[3].first == std::variant<Symbol, Time>(0.5));
	CHECK(run[3].second == Configuration<std::string>{{"s0", 1.5}, {"s1", 1.5}});
	CHECK(run[4].first == std::variant<Symbol, Time>("a"));
	CHECK(run[4].second == Configuration<std::string>{{"s0", 1.5}, {"s1", 0}, {"s1", 1.5}});
	CHECK(run[5].first == std::variant<Symbol, Time>(1.));
	CHECK(run[5].second == Configuration<std::string>{{"s0", 2.5}, {"s1", 1}, {"s1", 2.5}});
	CHECK(run[6].first == std::variant<Symbol, Time>("b"));
	CHECK(run[6].second == Configuration<std::string>{{"s0", 2.5}, {"s1", 1}, {"s1", 2.5}});

	// 4. option: disjunct `s1` on both b's
	run = runs[3];
	CHECK(run[3].first == std::variant<Symbol, Time>(0.5));
	CHECK(run[3].second == Configuration<std::string>{{"s0", 1.5}, {"s1", 1.5}});
	CHECK(run[4].first == std::variant<Symbol, Time>("a"));
	CHECK(run[4].second == Configuration<std::string>{{"s0", 1.5}, {"s1", 0}, {"s1", 1.5}});
	CHECK(run[5].first == std::variant<Symbol, Time>(1.));
	CHECK(run[5].second == Configuration<std::string>{{"s0", 2.5}, {"s1", 1}, {"s1", 2.5}});
	CHECK(run[6].first == std::variant<Symbol, Time>("b"));
	CHECK(run[6].second == Configuration<std::string>{{"s0", 2.5}, {"s1", 2.5}});

	SECTION("accepting the correct words")
	{
		CHECK(!ata.accepts_word({}));
		CHECK(ata.accepts_word({{"a", 1}, {"b", 2}}));
		CHECK(!ata.accepts_word({{"a", 1}, {"b", 1.5}}));
		CHECK(!ata.accepts_word({{"a", 1}, {"b", 1.9}}));
		CHECK(!ata.accepts_word({{"a", 1}, {"b", 1.9}}));
		CHECK(!ata.accepts_word({{"a", 1}, {"a", 2}}));
		CHECK(!ata.accepts_word({{"a", 1}, {"b", 2.5}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 0.5}, {"b", 1}}));
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}, {"b", 1}, {"b", 1.5}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 0.5}, {"b", 1}, {"a", 1}, {"b", 1.5}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 0.5}, {"b", 1}, {"b", 1.5}, {"a", 2.5}}));
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}, {"b", 1}, {"b", 1.5}, {"b", 2.0}}));
	}
}

TEST_CASE("Create an ATA with a non-string location type", "[ta]")
{
	std::set<Transition<unsigned int, std::string>> transitions;
	transitions.insert(Transition<unsigned int, std::string>(
	  0,
	  "a",
	  std::make_unique<ConjunctionFormula<unsigned int>>(
	    std::make_unique<LocationFormula<unsigned int>>(0),
	    std::make_unique<ResetClockFormula<unsigned int>>(
	      std::make_unique<LocationFormula<unsigned int>>(1)))));
	transitions.insert(Transition<unsigned int, std::string>(
	  1,
	  "a",
	  std::make_unique<ConjunctionFormula<unsigned int>>(
	    std::make_unique<LocationFormula<unsigned int>>(1),
	    std::make_unique<ClockConstraintFormula<unsigned int>>(
	      AtomicClockConstraintT<std::not_equal_to<Time>>(1.)))));
	AlternatingTimedAutomaton<unsigned int, std::string> ata({"a"},
	                                                         0,
	                                                         {0, 1},
	                                                         std::move(transitions));
	CHECK(!ata.accepts_word({}));
	CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}}));
	CHECK(!ata.accepts_word({{"a", 0}, {"a", 1}}));
	CHECK(ata.accepts_word({{"a", 0}, {"a", 1.1}, {"a", 2}}));
	CHECK(!ata.accepts_word({{"a", 0}, {"a", 1.1}, {"a", 2}, {"a", 3}}));
}

} // namespace
