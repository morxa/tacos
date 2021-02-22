/***************************************************************************
 *  test_print_mtl_formula.cpp - Test MTL Formula to string conversion
 *
 *  Created:   Mon 25 Jan 14:36:46 CET 2021
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

#include "mtl/MTLFormula.h"

#include <catch2/catch.hpp>
#include <sstream>

namespace {

using Formula = logic::MTLFormula<std::string>;
using AP      = logic::AtomicProposition<std::string>;
using logic::TimeInterval;

TEST_CASE("Print MTL formulas", "[print][mtl]")
{
	{
		std::stringstream s;
		s << Formula{AP{"a"}};
		CHECK(s.str() == "a");
	}
	{
		std::stringstream s;
		s << Formula(AP("a long atomic proposition"));
		CHECK(s.str() == "a long atomic proposition");
	}
	{
		std::stringstream s;
		s << (Formula{AP{"a"}} && Formula{AP{"b"}});
		CHECK(s.str() == "(a && b)");
	}
	{
		std::stringstream s;
		s << (Formula{AP{"a"}} || Formula{AP{"b"}});
		CHECK(s.str() == "(a || b)");
	}
	{
		std::stringstream s;
		s << (Formula(AP{"a"}).until(Formula{AP{"b"}}));
		CHECK(s.str() == "(a U b)");
	}
	{
		std::stringstream s;
		// TODO we should actually print the interval
		s << (Formula(AP{"a"}).until(Formula{AP{"b"}}, TimeInterval(1, 2)));
		CHECK(s.str() == "(a U b)");
	}
	{
		std::stringstream s;
		s << (Formula(AP{"a"}).dual_until(Formula{AP{"b"}}));
		CHECK(s.str() == "(a ~U b)");
	}
	{
		std::stringstream s;
		// TODO we should actually print the interval
		s << (Formula(AP{"a"}).dual_until(Formula{AP{"b"}}, TimeInterval(1, 2)));
		CHECK(s.str() == "(a ~U b)");
	}
}

} // namespace
