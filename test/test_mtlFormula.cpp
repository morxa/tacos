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

#include "mtl/MTLFormula.h"

#include <catch2/catch.hpp>
#include <iostream>

namespace {

TEST_CASE("Word boundaries", "[libmtl]")
{
	logic::AtomicProposition<std::string> a{"a"};
	{
		logic::MTLWord<std::string> word{};
		CHECK(!word.satisfies_at(a, 0));
	}
	{
		logic::MTLWord<std::string> word{{{{a}, 0}}};
		CHECK(word.satisfies_at(a, 0));
		CHECK(!word.satisfies_at(a, 1));
	}
}

TEST_CASE("Construction & simple satisfaction", "[libmtl]")
{
	logic::AtomicProposition<std::string> a{"a"};
	logic::AtomicProposition<std::string> b{"b"};
	logic::AtomicProposition<std::string> c{"c"};
	logic::AtomicProposition<std::string> copyA{a};

	REQUIRE(copyA == a);
	REQUIRE(copyA.ap_ == "a");

	auto                           word = logic::MTLWord<std::string>({{{a, b}, 0}});
	logic::MTLWord<std::string>    word2{{{a}, 1}, {{b}, 3}};
	logic::MTLFormula<std::string> phi1{a};
	logic::MTLFormula<std::string> phi2{b};
	logic::MTLFormula<std::string> phi3 = phi1 && phi2;
	logic::MTLFormula<std::string> phi4 = phi1.until(phi2, {1, 4});
	logic::MTLFormula<std::string> copyPhi1{phi1};

	REQUIRE(copyPhi1 == phi1);
	REQUIRE(phi1.get_operator() == logic::LOP::AP);
	REQUIRE(copyPhi1.get_operator() == logic::LOP::AP);
	REQUIRE(phi1.get_atomicProposition() == a);
	REQUIRE(copyPhi1.get_atomicProposition() == a);

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

TEST_CASE("MTL literals", "[libmtl]")
{
	CHECK(logic::MTLWord<std::string>({{{}, 0}}).satisfies_at(
	  logic::MTLFormula(logic::AtomicProposition<std::string>("true")), 0));
	// Word too short, does not matter that the formula is "true".
	CHECK(!logic::MTLWord<std::string>({{{}, 0}}).satisfies_at(
	  logic::MTLFormula(logic::AtomicProposition<std::string>("true")), 1));
	CHECK(!logic::MTLWord<std::string>({{{}, 0}}).satisfies_at(
	  logic::MTLFormula(logic::AtomicProposition<std::string>("false")), 0));
}

TEST_CASE("Dual until", "[libmtl]")
{
	logic::AtomicProposition<std::string> a{"a"};
	logic::AtomicProposition<std::string> b{"b"};

	// build two formulas for comparison
	logic::MTLFormula neg_until        = logic::MTLFormula(!a).until(logic::MTLFormula(!b));
	logic::MTLFormula double_neg_until = !logic::MTLFormula(!a).until(logic::MTLFormula(!b));
	logic::MTLFormula dual_until       = logic::MTLFormula(a).dual_until(logic::MTLFormula(b));

	logic::MTLFormula until = logic::MTLFormula(a).until(logic::MTLFormula(b));

	logic::MTLWord<std::string> word1{{{a}, 2}, {{b}, 3}};
	REQUIRE(word1.satisfies(until));

	logic::MTLWord<std::string> word2{{{a}, 1}, {{{""}}, 2}, {{b}, 3}};
	REQUIRE(!word2.satisfies(until));
	REQUIRE(word2.satisfies(neg_until));

	logic::MTLWord<std::string> word3{{{a}, 1}};
	REQUIRE(!word3.satisfies(until));

	logic::MTLWord<std::string> word4{{{b}, 10}};                                 // should hold
	logic::MTLWord<std::string> word5{{{b, a}, 10}, {{b}, 11}};                   // should hold
	logic::MTLWord<std::string> word6{{{a}, 1}, {{b}, 10}, {{a}, 10}, {{b}, 11}}; // should not hold

	REQUIRE(word4.satisfies(dual_until));
	REQUIRE(word5.satisfies(dual_until));
	REQUIRE(!word6.satisfies(dual_until));
	REQUIRE(
	  logic::MTLWord<std::string>{{{b}, 1}, {{b}, 2}, {{b}, 3}, {{b}, 4}, {{a, b}, 5}}.satisfies(
	    dual_until));
	REQUIRE(word1.satisfies(double_neg_until) == word1.satisfies(dual_until));
	REQUIRE(word2.satisfies(double_neg_until) == word2.satisfies(dual_until));
	REQUIRE(word3.satisfies(double_neg_until) == word3.satisfies(dual_until));
	REQUIRE(word4.satisfies(double_neg_until) == word4.satisfies(dual_until));
	REQUIRE(word5.satisfies(double_neg_until) == word5.satisfies(dual_until));
}

TEST_CASE("To positive normal form", "[libmtl]")
{
	logic::AtomicProposition<std::string> a{"a"};
	logic::AtomicProposition<std::string> b{"b"};

	auto na         = !a;
	auto nb         = !b;
	auto land       = a && b;
	auto lor        = a || b;
	auto nland      = !land;
	auto nlor       = !lor;
	auto until      = logic::MTLFormula(a).until(b);
	auto dual_until = logic::MTLFormula(a).dual_until(b);

	REQUIRE(land.to_positive_normal_form() == land);
	REQUIRE(lor.to_positive_normal_form() == lor);
	REQUIRE(nland.to_positive_normal_form() == (na || nb));
	REQUIRE((!nland).to_positive_normal_form() == (a && b));
	REQUIRE(nlor.to_positive_normal_form() == (na && nb));
	REQUIRE(((!until).to_positive_normal_form()) == na.dual_until(nb));
	REQUIRE(((!dual_until).to_positive_normal_form()) == na.until(nb));
}

TEST_CASE("Comparison operators", "[libmtl]")
{
	logic::AtomicProposition a{std::string("a")};
	logic::AtomicProposition b{std::string("b")};
	logic::AtomicProposition c{std::string("c")};

	logic::MTLFormula phi1{a};
	logic::MTLFormula phi2{b};
	logic::MTLFormula phi3 = phi1 && phi2;
	logic::MTLFormula phi4 = phi1.until(phi2, {1, 4});
	logic::MTLFormula phi5{c};

	CHECK(a == a);
	CHECK(a != b);
	CHECK(a < b);

	CHECK(phi1 == phi1);
	CHECK(phi1 != phi2);
	CHECK(phi1 < phi2);
	CHECK(phi1 <= phi2);
	CHECK(!(phi2 <= phi1));
	CHECK(!(phi1 >= phi2));
	CHECK(phi2 >= phi1);
	CHECK(phi4 != phi1);
	CHECK(phi1 > phi4);
	CHECK(phi3 < (phi1 && phi5));
}

TEST_CASE("Get subformulas of type", "[libmtl]")
{
	logic::AtomicProposition<std::string> a{"a"};
	logic::AtomicProposition<std::string> b{"b"};
	logic::AtomicProposition<std::string> c{"c"};

	logic::MTLFormula phi1{a};
	logic::MTLFormula phi2{b};
	logic::MTLFormula phi4 = phi1.until(phi2, {1, 4});
	auto              phi5 = phi4 && phi1;
	auto              phi6 = logic::MTLFormula(c) || phi5;

	auto atomicPropositions = phi6.get_subformulas_of_type(logic::LOP::AP);
	REQUIRE(atomicPropositions.size() == std::size_t(3));
	REQUIRE(std::find(atomicPropositions.begin(), atomicPropositions.end(), phi1)
	        != atomicPropositions.end());
	REQUIRE(std::find(atomicPropositions.begin(), atomicPropositions.end(), phi2)
	        != atomicPropositions.end());
	REQUIRE(std::find(atomicPropositions.begin(), atomicPropositions.end(), logic::MTLFormula(c))
	        != atomicPropositions.end());
	REQUIRE(std::find(atomicPropositions.begin(),
	                  atomicPropositions.end(),
	                  logic::MTLFormula<std::string>({"not_contained"}))
	        == atomicPropositions.end());

	auto conjunctions = phi6.get_subformulas_of_type(logic::LOP::LAND);

	REQUIRE(conjunctions.size() == std::size_t(1));
	REQUIRE(std::find(conjunctions.begin(), conjunctions.end(), phi5) != conjunctions.end());

	auto alphabet = phi6.get_alphabet();
	REQUIRE(std::set<logic::AtomicProposition<std::string>>({{"a"}, {"b"}, {"c"}}) == alphabet);
}

} // namespace
