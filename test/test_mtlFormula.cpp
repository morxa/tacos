/***************************************************************************
 *  test_mtlFormula.cpp - Test MTLFormula implementation
 *
 *  Created: 4 June 2020
 *  Copyright  2020  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
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
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include <mtl/MTLFormula.h>

#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("Construction & simple satisfaction", "[libmtl]")
{
	logic::AtomicProposition a{"a"};
	logic::AtomicProposition b{"b"};
	logic::AtomicProposition c{"c"};
	logic::AtomicProposition copyA{a};

	REQUIRE(copyA == a);
	REQUIRE(copyA.ap_ == "a");

	auto              word = logic::MTLWord({{{a, b}, 0}});
	logic::MTLWord    word2{{{a}, 1}, {{b}, 3}};
	logic::MTLFormula phi1{a};
	logic::MTLFormula phi2{b};
	logic::MTLFormula phi3 = phi1 && phi2;
	logic::MTLFormula phi4 = phi1.until(phi2, {1, 4});

	REQUIRE(word.satisfies_at({true}, 0));
	REQUIRE(!word.satisfies_at({false}, 0));
	REQUIRE(word.satisfies_at({{"true"}}, 0));
	REQUIRE(!word.satisfies_at({{"false"}}, 0));
	REQUIRE(word.satisfies_at(phi1, 0));
	REQUIRE(word.satisfies_at(phi2, 0));
	REQUIRE(!word.satisfies_at({c}, 0));
	REQUIRE(word.satisfies_at(phi3, 0));
	REQUIRE(word.satisfies_at(a || b, 0));
	REQUIRE(word.satisfies_at(a && b, 0));
	REQUIRE(!word.satisfies_at(a && b && c, 0));
	REQUIRE(word2.satisfies(phi4));
	REQUIRE(!word2.satisfies(phi1.until(phi2, {1, 1})));
}

TEST_CASE("Comparison operators", "[libmtl]")
{
	logic::AtomicProposition a{"a"};
	logic::AtomicProposition b{"b"};
	logic::AtomicProposition c{"c"};

	logic::MTLFormula phi1{a};
	logic::MTLFormula phi2{b};
	logic::MTLFormula phi3 = phi1 && phi2;
	logic::MTLFormula phi4 = phi1.until(phi2, {1, 4});

	REQUIRE(a == a);
	REQUIRE(a != b);
	REQUIRE(a < b);

	REQUIRE(phi1 == phi1);
	REQUIRE(phi1 != phi2);
	REQUIRE(phi1 < phi2);
	REQUIRE(phi4 != phi1);
	REQUIRE(phi1 > phi4);
}

TEST_CASE("Get subformulas of type", "[libmtl]")
{
	logic::AtomicProposition a{"a"};
	logic::AtomicProposition b{"b"};
	logic::AtomicProposition c{"c"};

	logic::MTLFormula phi1{a};
	logic::MTLFormula phi2{b};
	logic::MTLFormula phi3 = phi1 && phi2;
	logic::MTLFormula phi4 = phi1.until(phi2, {1, 4});
	auto              phi5 = phi4 && phi1;
	auto              phi6 = logic::MTLFormula(c) || phi5;
}
