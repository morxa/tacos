/***************************************************************************
 *  test_print_ata_formula.cpp - Test conversion of formulas to strings
 *
 *  Created:   Fri  4 Dec 14:30:50 CET 2020
 *  Copyright  2020 Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



#include "automata/ata_formula.h"
#include "automata/automata.h"

#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <memory>
#include <sstream>
#include <string>

namespace {

using namespace tacos;
using namespace automata::ata;

TEST_CASE("Print a TrueFormula", "[print][ata_formula]")
{
	TrueFormula<std::string> f{};
	std::stringstream        s;
	s << f;
	REQUIRE(s.str() == u8"⊤");
}

TEST_CASE("Print a FalseFormula", "[print][ata_formula]")
{
	FalseFormula<std::string> f{};
	std::stringstream         s;
	s << f;
	REQUIRE(s.str() == u8"⊥");
}

TEST_CASE("Print a location formula", "[print][ata_formula]")
{
	{
		LocationFormula<std::string> f{"s0"};
		std::stringstream            s;
		s << f;
		REQUIRE(s.str() == "s0");
	}
	{
		LocationFormula<int> f{5};
		std::stringstream    s;
		s << f;
		REQUIRE(s.str() == "5");
	}
}

TEST_CASE("Print a clock constraint formula", "[print][ata_formula]")
{
	{
		ClockConstraintFormula<std::string> f(automata::AtomicClockConstraintT<std::less<Time>>(1));
		std::stringstream                   s;
		s << f;
		REQUIRE(s.str() == "x < 1");
	}
	{
		ClockConstraintFormula<std::string> f(
		  automata::AtomicClockConstraintT<std::less_equal<Time>>(2));
		std::stringstream s;
		s << f;
		REQUIRE(s.str() == u8"x ≤ 2");
	}
	{
		ClockConstraintFormula<std::string> f(automata::AtomicClockConstraintT<std::equal_to<Time>>(3));
		std::stringstream                   s;
		s << f;
		REQUIRE(s.str() == "x = 3");
	}
	{
		ClockConstraintFormula<std::string> f(
		  automata::AtomicClockConstraintT<std::not_equal_to<Time>>(4));
		std::stringstream s;
		s << f;
		REQUIRE(s.str() == u8"x ≠ 4");
	}
	{
		ClockConstraintFormula<std::string> f(
		  automata::AtomicClockConstraintT<std::greater_equal<Time>>(5));
		std::stringstream s;
		s << f;
		REQUIRE(s.str() == u8"x ≥ 5");
	}
	{
		ClockConstraintFormula<std::string> f(automata::AtomicClockConstraintT<std::greater<Time>>(6));
		std::stringstream                   s;
		s << f;
		REQUIRE(s.str() == "x > 6");
	}
}

TEST_CASE("Print a conjunction formula", "[print][ata_formula]")
{
	// A simple conjunction
	{
		ConjunctionFormula<std::string> f{std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<LocationFormula<std::string>>("s1")};
		std::stringstream               s;
		s << f;
		REQUIRE(s.str() == u8"(s0 ∧ s1)");
	}
	// First conjunct is a nested conjunction
	{
		ConjunctionFormula<std::string> f{std::make_unique<ConjunctionFormula<std::string>>(
		                                    std::make_unique<LocationFormula<std::string>>("s0"),
		                                    std::make_unique<LocationFormula<std::string>>("s1")),
		                                  std::make_unique<LocationFormula<std::string>>("s2")};
		std::stringstream               s;
		s << f;
		REQUIRE(s.str() == u8"((s0 ∧ s1) ∧ s2)");
	}
	// Second conjunct is a nested conjunction
	{
		ConjunctionFormula<std::string> f{std::make_unique<LocationFormula<std::string>>("s2"),
		                                  std::make_unique<ConjunctionFormula<std::string>>(
		                                    std::make_unique<LocationFormula<std::string>>("s0"),
		                                    std::make_unique<LocationFormula<std::string>>("s1"))};
		std::stringstream               s;
		s << f;
		REQUIRE(s.str() == u8"(s2 ∧ (s0 ∧ s1))");
	}
}

TEST_CASE("Print a disjunction formula", "[print][ata_formula]")
{
	// A simple disjunction
	{
		DisjunctionFormula<std::string> f{std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<LocationFormula<std::string>>("s1")};
		std::stringstream               s;
		s << f;
		REQUIRE(s.str() == u8"(s0 ∨ s1)");
	}
	// First disjunct is a nested conjunction
	{
		DisjunctionFormula<std::string> f{std::make_unique<ConjunctionFormula<std::string>>(
		                                    std::make_unique<LocationFormula<std::string>>("s0"),
		                                    std::make_unique<LocationFormula<std::string>>("s1")),
		                                  std::make_unique<LocationFormula<std::string>>("s2")};
		std::stringstream               s;
		s << f;
		REQUIRE(s.str() == u8"((s0 ∧ s1) ∨ s2)");
	}
}

TEST_CASE("Print a ResetClockFormula", "[print][ata_formula]")
{
	{
		ResetClockFormula<std::string> f{std::make_unique<LocationFormula<std::string>>("s0")};
		std::stringstream              s;
		s << f;
		REQUIRE(s.str() == "x.s0");
	}
	// Use a nested conjunction
	{
		ResetClockFormula<std::string> f{std::make_unique<ConjunctionFormula<std::string>>(
		  std::make_unique<LocationFormula<std::string>>("s0"),
		  std::make_unique<LocationFormula<std::string>>("s1"))};
		std::stringstream              s;
		s << f;
		REQUIRE(s.str() == u8"x.(s0 ∧ s1)");
	}
}

} // namespace
