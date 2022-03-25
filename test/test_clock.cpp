/***************************************************************************
 *  test_clock.cpp - Test the Clock class
 *
 *  Created:   Tue  9 Feb 12:48:44 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include "automata/automata.h"

#include <catch2/catch_test_macros.hpp>

namespace {

using namespace tacos;
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
