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

#include <libmtl/MTLFormula.h>
#include <libmtl/operators.h>

#include <catch2/catch.hpp>

TEST_CASE("Construction & simple satisfaction", "[libmtl]")
{
	logic::AtomicProposition a{"a"};
	logic::AtomicProposition b{"b"};
	logic::AtomicProposition c{"c"};

	auto              word = logic::MTLWord({{{a, b}, 0}});
	logic::MTLWord    word2{{{a}, 1}, {{b}, 3}};
	logic::MTLFormula phi1{a};
	logic::MTLFormula phi2{b};
	logic::MTLFormula phi3 = phi1 && phi2;
	logic::MTLFormula phi4 = phi1.until(phi2, {1, 4});

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
