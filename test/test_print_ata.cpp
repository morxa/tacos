/***************************************************************************
 *  test_print_ata.cpp - Tests for printing an ATA
 *
 *  Created:   Mon  7 Dec 15:44:03 CET 2020
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
 *  Read the full text in the LICENSE.md file.
 */

#include <ta/ata.h>
#include <ta/ata_formula.h>

#include <catch2/catch.hpp>
#include <iomanip>
#include <memory>

namespace {

using automata::ata::AlternatingTimedAutomaton;
using automata::ata::ConjunctionFormula;
using automata::ata::LocationFormula;
using automata::ata::State;
using automata::ata::Transition;

TEST_CASE("Print a state", "[ata]")
{
	{
		State<std::string> state{"s0", 0};
		{
			std::stringstream s;
			s << state;
			REQUIRE(s.str() == "(s0, 0)");
		}
		{
			std::stringstream s;
			s << std::fixed;
			s << std::setprecision(2);
			s << state;
			REQUIRE(s.str() == "(s0, 0.00)");
		}
	}
	{
		State<std::string> state{"s0", 1.2345};
		{
			std::stringstream s;
			s << std::fixed;
			s << std::setprecision(2);
			s << state;
			REQUIRE(s.str() == "(s0, 1.23)");
		}
	}
}

TEST_CASE("Print a transition", "[ata]")
{
	{
		Transition<std::string, std::string> t("s0",
		                                       "a",
		                                       std::make_unique<LocationFormula<std::string>>("s1"));
		std::stringstream                    s;
		s << t;
		REQUIRE(s.str() == "s0 -- a --> s1");
	}
	{
		Transition<std::string, std::string> t("s0",
		                                       "a",
		                                       std::make_unique<ConjunctionFormula<std::string>>(
		                                         std::make_unique<LocationFormula<std::string>>("s0"),
		                                         std::make_unique<LocationFormula<std::string>>("s1")));
		std::stringstream                    s;
		s << t;
		REQUIRE(s.str() == "s0 -- a --> (s0 ∧ s1)");
	}
}

TEST_CASE("Print a simple ATA", "[ata]")
{
	std::set<Transition<std::string, std::string>> transitions;
	transitions.insert(Transition<std::string, std::string>(
	  "s0", "a", std::make_unique<LocationFormula<std::string>>("s0")));
	transitions.insert(Transition<std::string, std::string>(
	  "s0", "b", std::make_unique<LocationFormula<std::string>>("s1")));
	AlternatingTimedAutomaton<std::string, std::string> ata({"a"},
	                                                        "s0",
	                                                        {"s0"},
	                                                        std::move(transitions));
	std::stringstream                                   s;
	s << ata;
	REQUIRE(s.str()
	        == "Alphabet: {a}, initial location: s0, final locations: {s0}, transitions:\n"
	           "  s0 -- a --> s0\n"
	           "  s0 -- b --> s1");
}

TEST_CASE("Print a run", "[ata]")
{
	std::set<Transition<std::string, std::string>> transitions;
	transitions.insert(Transition<std::string, std::string>(
	  "s0", "a", std::make_unique<LocationFormula<std::string>>("s0")));
	transitions.insert(Transition<std::string, std::string>(
	  "s0", "b", std::make_unique<LocationFormula<std::string>>("s1")));
	AlternatingTimedAutomaton<std::string, std::string> ata({"a"},
	                                                        "s0",
	                                                        {"s0"},
	                                                        std::move(transitions));
	{
		const auto        runs = ata.make_symbol_transition({{}}, "a");
		std::stringstream s;
		REQUIRE(runs.size() == 1);
		s << runs[0];
		REQUIRE(s.str() == " → a → { (s0, 0) }");
	}
	{
		const auto runs =
		  ata.make_symbol_transition(ata.make_time_transition(ata.make_symbol_transition({{}}, "a"), 1),
		                             "b");
		std::stringstream s;
		REQUIRE(runs.size() == 1);
		s << runs[0];
		REQUIRE(s.str() == " → a → { (s0, 0) } ⇢ 1 ⇢ { (s0, 1) } → b → { (s1, 1) }");
	}
}

TEST_CASE("Print a run with multiple possible configurations", "[ata]")
{
	std::set<Transition<std::string, std::string>> transitions;
	transitions.insert(
	  Transition<std::string, std::string>("s0",
	                                       "a",
	                                       std::make_unique<ConjunctionFormula<std::string>>(
	                                         std::make_unique<LocationFormula<std::string>>("s0"),
	                                         std::make_unique<LocationFormula<std::string>>("s1"))));
	AlternatingTimedAutomaton<std::string, std::string> ata({"a"},
	                                                        "s0",
	                                                        {"s0"},
	                                                        std::move(transitions));
	{
		const auto        runs = ata.make_symbol_transition({{}}, "a");
		std::stringstream s;
		REQUIRE(runs.size() == 1);
		s << runs[0];
		REQUIRE(s.str() == " → a → { (s0, 0), (s1, 0) }");
	}
}

} // namespace
