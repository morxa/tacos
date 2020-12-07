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
} // namespace
