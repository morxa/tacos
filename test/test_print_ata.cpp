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

#include "ta/ata_formula.h"

#include <ta/ata.h>

#include <catch2/catch.hpp>
#include <memory>

namespace {

using automata::ata::AlternatingTimedAutomaton;
using automata::ata::ConjunctionFormula;
using automata::ata::LocationFormula;
using automata::ata::Transition;

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
} // namespace
