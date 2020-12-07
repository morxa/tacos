/***************************************************************************
 *  test_print_ata_formula.cpp - Test conversion of formulas to strings
 *
 *  Created:   Fri  4 Dec 14:30:50 CET 2020
 *  Copyright  2020 Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#include "ta/automata.h"

#include <ta/ata_formula.h>

#include <catch2/catch.hpp>
#include <functional>
#include <sstream>
#include <string>

namespace {

using namespace automata::ata;

TEST_CASE("Print a TrueFormula", "[ata_formula]")
{
	TrueFormula<std::string> f{};
	std::stringstream        s;
	s << f;
	REQUIRE(s.str() == "⊤");
}

TEST_CASE("Print a FalseFormula", "[ata_formula]")
{
	FalseFormula<std::string> f{};
	std::stringstream         s;
	s << f;
	REQUIRE(s.str() == "⊥");
}

TEST_CASE("Print a location formula", "[ata_formula]")
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

TEST_CASE("Print a clock constraint formula", "[ata_formula]")
{
	{
		ClockConstraintFormula<std::string> f(
		  automata::AtomicClockConstraintT<std::less<automata::Time>>(1));
		std::stringstream s;
		s << f;
		REQUIRE(s.str() == "x < 1");
	}
	{
		ClockConstraintFormula<std::string> f(
		  automata::AtomicClockConstraintT<std::less_equal<automata::Time>>(2));
		std::stringstream s;
		s << f;
		REQUIRE(s.str() == "x ≤ 2");
	}
	{
		ClockConstraintFormula<std::string> f(
		  automata::AtomicClockConstraintT<std::equal_to<automata::Time>>(3));
		std::stringstream s;
		s << f;
		REQUIRE(s.str() == "x = 3");
	}
	{
		ClockConstraintFormula<std::string> f(
		  automata::AtomicClockConstraintT<std::not_equal_to<automata::Time>>(4));
		std::stringstream s;
		s << f;
		REQUIRE(s.str() == "x ≠ 4");
	}
	{
		ClockConstraintFormula<std::string> f(
		  automata::AtomicClockConstraintT<std::greater_equal<automata::Time>>(5));
		std::stringstream s;
		s << f;
		REQUIRE(s.str() == "x ≥ 5");
	}
	{
		ClockConstraintFormula<std::string> f(
		  automata::AtomicClockConstraintT<std::greater<automata::Time>>(6));
		std::stringstream s;
		s << f;
		REQUIRE(s.str() == "x > 6");
	}
}
} // namespace
