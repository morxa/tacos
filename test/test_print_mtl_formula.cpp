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
#include "utilities/Interval.h"

#include <catch2/catch_test_macros.hpp>
#include <sstream>

namespace {

using namespace tacos;

using logic::TimeInterval;
using utilities::arithmetic::BoundType;

TEST_CASE("Print MTL formulas", "[print][mtl]")
{
	using Formula = logic::MTLFormula<std::string>;
	using AP      = logic::AtomicProposition<std::string>;
	std::stringstream s;
	SECTION("atom")
	{
		s << Formula{AP{"a"}};
		CHECK(s.str() == "a");
	}
	SECTION("long atom")
	{
		s << Formula(AP("a long atomic proposition"));
		CHECK(s.str() == "a long atomic proposition");
	}
	SECTION("conjunction")
	{
		s << (Formula{AP{"a"}} && Formula{AP{"b"}});
		CHECK(s.str() == "(a ∧ b)");
	}
	SECTION("empty conjunction")
	{
		s << Formula::create_conjunction({});
		CHECK(s.str() == u8"⊤");
	}
	SECTION("conjunction with a single conjunct")
	{
		s << Formula::create_conjunction({AP{"a"}});
		CHECK(s.str() == "a");
	}
	SECTION("conjunction with three conjuncts")
	{
		s << Formula::create_conjunction({AP{"a"}, AP{"b"}, AP{"c"}});
		CHECK(s.str() == "(a ∧ b ∧ c)");
	}
	SECTION("disjunction")
	{
		s << (Formula{AP{"a"}} || Formula{AP{"b"}});
		CHECK(s.str() == "(a ∨ b)");
	}
	SECTION("empty disjunction")
	{
		s << Formula::create_disjunction({});
		CHECK(s.str() == u8"⊥");
	}
	SECTION("disjunction with a single conjunct")
	{
		s << Formula::create_disjunction({AP{"a"}});
		CHECK(s.str() == "a");
	}
	SECTION("disjunction with three conjuncts")
	{
		s << Formula::create_disjunction({AP{"a"}, AP{"b"}, AP{"c"}});
		CHECK(s.str() == "(a ∨ b ∨ c)");
	}
	SECTION("until")
	{
		s << (Formula(AP{"a"}).until(Formula{AP{"b"}}));
		CHECK(s.str() == "(a U b)");
	}
	SECTION("until with time bounds")
	{
		s << (Formula(AP{"a"}).until(Formula{AP{"b"}}, TimeInterval(1, 2)));
		CHECK(s.str() == "(a U[1, 2] b)");
	}
	SECTION("until with strict time bound")
	{
		s << (Formula(AP{"a"}).until(Formula{AP{"b"}},
		                             TimeInterval(1, BoundType::STRICT, 3, BoundType::WEAK)));
		CHECK(s.str() == "(a U(1, 3] b)");
	}
	SECTION("dual until")
	{
		s << (Formula(AP{"a"}).dual_until(Formula{AP{"b"}}));
		CHECK(s.str() == "(a ~U b)");
	}
	SECTION("dual until with time bounds")
	{
		s << (Formula(AP{"a"}).dual_until(Formula{AP{"b"}}, TimeInterval(1, 2)));
		CHECK(s.str() == "(a ~U[1, 2] b)");
	}
	SECTION("dual until with strict time bound")
	{
		s << (Formula(AP{"a"}).dual_until(Formula{AP{"b"}},
		                                  TimeInterval(3, BoundType::WEAK, 5, BoundType::STRICT)));
		CHECK(s.str() == "(a ~U[3, 5) b)");
	}
}

TEST_CASE("Print MTL formulas over vectors", "[print][mtl]")
{
	using Formula = logic::MTLFormula<std::vector<std::string>>;
	using AP      = logic::AtomicProposition<std::vector<std::string>>;
	{
		std::stringstream s;
		s << Formula{AP{{"s1, s2"}}};
		CHECK(s.str() == "(s1, s2)");
	}
}

} // namespace
