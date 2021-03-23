/***************************************************************************
 *  test_clock.cpp - Test the Clock class
 *
 *  Created:   Tue  9 Feb 12:48:44 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#include "automata/automata.h"

#include <catch2/catch_test_macros.hpp>

namespace {

using automata::Clock;
using automata::Time;

TEST_CASE("Clock initialization", "[automata]")
{
	CHECK(Clock{}.get_valuation() == 0);
	CHECK(Clock{5}.get_valuation() == 5);
}

TEST_CASE("Clock time progression", "[automata]")
{
	Clock c;
	CHECK(c.get_valuation() == 0);
	c.tick(2.5);
	CHECK(c.get_valuation() == 2.5);
	c.reset();
	CHECK(c.get_valuation() == 0);
}

TEST_CASE("Clock implicit conversion to time", "[automata]")
{
	CHECK(Clock{} == Time{0});
	CHECK(Clock{0.1} == Time{0.1});
}

} // namespace
