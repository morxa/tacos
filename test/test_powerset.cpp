/***************************************************************************
 *  test_powerset.cpp - Test powerset generation
 *
 *  Created:   Wed 19 Jan 14:49:40 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#include "utilities/powerset.h"

#include <catch2/catch_test_macros.hpp>
#include <set>
#include <string>

using tacos::utilities::construct_powerset;

TEST_CASE("Power set", "[utilities]")
{
	std::set<std::string> input{"a", "b"};
	CHECK(construct_powerset(input) == std::set<std::set<std::string>>{{}, {"a"}, {"b"}, {"a", "b"}});
	CHECK(construct_powerset(std::set<std::string>{}) == std::set<std::set<std::string>>{{}});
}
