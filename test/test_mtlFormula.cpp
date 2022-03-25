/***************************************************************************
 *  test_mtlFormula.cpp - Test MTLFormula implementation
 *
 *  Created: 4 June 2020
 *  Copyright  2020  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



#include "mtl/MTLFormula.h"
#include "utilities/Interval.h"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <vector>

namespace {

using namespace tacos;

using logic::finally;
using logic::globally;

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

TEST_CASE("MTL construction from a vector of operands", "[libmtl]")
{
	logic::AtomicProposition<std::string> a{"a"};
	logic::AtomicProposition<std::string> b{"b"};
	logic::AtomicProposition<std::string> c{"c"};
	// Conjunction
	const auto conjunction = logic::MTLFormula<std::string>::create_conjunction({a, b, c});
	CAPTURE(conjunction);
	CHECK(logic::MTLWord<std::string>{{{a, b, c}, 0}}.satisfies(conjunction));
	CHECK(!logic::MTLWord<std::string>{{{a, b}, 0}}.satisfies(conjunction));
	CHECK(!logic::MTLWord<std::string>{{{a, c}, 0}}.satisfies(conjunction));
	CHECK(!logic::MTLWord<std::string>{{{b, c}, 0}}.satisfies(conjunction));
	CHECK(!logic::MTLWord<std::string>{{{a}, 0}}.satisfies(conjunction));
	CHECK(!logic::MTLWord<std::string>{{{b}, 0}}.satisfies(conjunction));
	CHECK(!logic::MTLWord<std::string>{{{c}, 0}}.satisfies(conjunction));
	CHECK(!logic::MTLWord<std::string>{{{}, 0}}.satisfies(conjunction));
	// Negate the conjunction.
	CHECK(!logic::MTLWord<std::string>{{{a, b, c}, 0}}.satisfies(!conjunction));
	CHECK(logic::MTLWord<std::string>{{{a, b}, 0}}.satisfies(!conjunction));
	CHECK(logic::MTLWord<std::string>{{{a, c}, 0}}.satisfies(!conjunction));
	CHECK(logic::MTLWord<std::string>{{{b, c}, 0}}.satisfies(!conjunction));
	CHECK(logic::MTLWord<std::string>{{{a}, 0}}.satisfies(!conjunction));
	CHECK(logic::MTLWord<std::string>{{{b}, 0}}.satisfies(!conjunction));
	CHECK(logic::MTLWord<std::string>{{{c}, 0}}.satisfies(!conjunction));
	CHECK(logic::MTLWord<std::string>{{{}, 0}}.satisfies(!conjunction));
	// Empty conjunction
	CHECK(logic::MTLWord<std::string>{{{a, b, c}, 0}}.satisfies(
	  logic::MTLFormula<std::string>::create_conjunction({})));
	// Disjunction
	const auto disjunction = logic::MTLFormula<std::string>::create_disjunction({a, b, c});
	CAPTURE(disjunction);
	CHECK(logic::MTLWord<std::string>{{{a, b, c}, 0}}.satisfies(disjunction));
	CHECK(logic::MTLWord<std::string>{{{a, b}, 0}}.satisfies(disjunction));
	CHECK(logic::MTLWord<std::string>{{{a, c}, 0}}.satisfies(disjunction));
	CHECK(logic::MTLWord<std::string>{{{b, c}, 0}}.satisfies(disjunction));
	CHECK(logic::MTLWord<std::string>{{{a}, 0}}.satisfies(disjunction));
	CHECK(logic::MTLWord<std::string>{{{b}, 0}}.satisfies(disjunction));
	CHECK(logic::MTLWord<std::string>{{{c}, 0}}.satisfies(disjunction));
	CHECK(!logic::MTLWord<std::string>{{{}, 0}}.satisfies(disjunction));
	// Negate the disjunction.
	CHECK(!logic::MTLWord<std::string>{{{a, b, c}, 0}}.satisfies(!disjunction));
	CHECK(!logic::MTLWord<std::string>{{{a, b}, 0}}.satisfies(!disjunction));
	CHECK(!logic::MTLWord<std::string>{{{a, c}, 0}}.satisfies(!disjunction));
	CHECK(!logic::MTLWord<std::string>{{{b, c}, 0}}.satisfies(!disjunction));
	CHECK(!logic::MTLWord<std::string>{{{a}, 0}}.satisfies(!disjunction));
	CHECK(!logic::MTLWord<std::string>{{{b}, 0}}.satisfies(!disjunction));
	CHECK(!logic::MTLWord<std::string>{{{c}, 0}}.satisfies(!disjunction));
	CHECK(logic::MTLWord<std::string>{{{}, 0}}.satisfies(!disjunction));
	// Empty disjunction
	CHECK(!logic::MTLWord<std::string>{{{a, b, c}, 0}}.satisfies(
	  logic::MTLFormula<std::string>::create_disjunction({})));
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

TEST_CASE("MTL Formula comparison operators", "[libmtl]")
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

	CHECK(phi1.until(phi2)
	      != phi1.until(phi2, utilities::arithmetic::Interval<logic::TimePoint>{0, 1}));

	CHECK(phi1.dual_until(phi2)
	      != phi1.dual_until(phi2, utilities::arithmetic::Interval<logic::TimePoint>{1, 2}));
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

TEST_CASE("MTL Finally and Globally", "[libmtl]")
{
	using MTLFormula = logic::MTLFormula<std::string>;
	using AP         = logic::AtomicProposition<std::string>;
	using MTLWord    = logic::MTLWord<std::string>;
	using logic::TimeInterval;
	const AP         a{"a"};
	const AP         b{"b"};
	const MTLFormula f_a{a};
	const MTLFormula f_b{b};

	CHECK(finally(f_a) == MTLFormula::TRUE().until(a));
	CHECK(globally(f_a) == !(MTLFormula::TRUE().until(!a)));
	CHECK(MTLWord({{{b}, 0}, {{b}, 1}, {{a}, 2}}).satisfies(finally(f_a, TimeInterval{0, 2})));
	CHECK(MTLWord({{{b}, 0}, {{b}, 1}, {{a}, 2}}).satisfies(finally(f_a, TimeInterval{0, 2})));
	CHECK(!MTLWord({{{b}, 0}, {{b}, 1}, {{a}, 3}}).satisfies(finally(f_a, TimeInterval{0, 2})));
	CHECK(MTLWord({{{b}, 0}, {{b}, 1}, {{a}, 3}}).satisfies(globally(f_b, TimeInterval{0, 1})));
	CHECK(!MTLWord({{{b}, 0}, {{b}, 1}, {{a}, 3}}).satisfies(globally(f_b, TimeInterval{0, 3})));
	CHECK(!MTLWord({{{b}, 0}, {{b}, 1}, {{a}, 3}}).satisfies(globally(f_b)));
}

TEST_CASE("MTL formulas over vectors", "[mtl]")
{
	using MTLFormula = logic::MTLFormula<std::vector<std::string>>;
	using AP         = logic::AtomicProposition<std::vector<std::string>>;
	using MTLWord    = logic::MTLWord<std::vector<std::string>>;
	MTLFormula a{AP{{"a1", "a2"}}};
	MTLFormula b{AP{{"b1", "b2"}}};
	CHECK(MTLWord({{{AP{{"a1", "a2"}}}, 0}}).satisfies(a));
	CHECK(!MTLWord({{{AP{{"a1", "a2"}}}, 0}}).satisfies(b));
	CHECK(MTLWord({{{AP{{"a1", "a2"}}}, 0}}).satisfies(a || b));
}

TEST_CASE("Get maximal region index of an MTL formula", "[libmtl]")
{
	using MTLFormula = logic::MTLFormula<std::string>;
	using AP         = logic::AtomicProposition<std::string>;
	using logic::TimeInterval;
	const auto a = MTLFormula{AP{"a"}};
	const auto b = MTLFormula{AP{"b"}};
	using utilities::arithmetic::BoundType;

	CHECK(a.get_maximal_region_index() == 1);
	CHECK(MTLFormula::TRUE().get_maximal_region_index() == 1);
	CHECK(MTLFormula::FALSE().get_maximal_region_index() == 1);
	CHECK(a.until(b).get_maximal_region_index() == 1);
	CHECK(a.dual_until(b).get_maximal_region_index() == 1);
	CHECK(finally(a).get_maximal_region_index() == 1);
	CHECK(globally(a).get_maximal_region_index() == 1);
	CHECK(a.until(b, TimeInterval{1, 2}).get_maximal_region_index() == 5);
	CHECK(a.dual_until(b, TimeInterval{3, BoundType::WEAK, 10, BoundType::INFTY})
	        .get_maximal_region_index()
	      == 7);
	CHECK(
	  finally(a, TimeInterval{10, BoundType::INFTY, 10, BoundType::STRICT}).get_maximal_region_index()
	  == 21);
	CHECK(globally(a, TimeInterval{11, BoundType::STRICT, 10, BoundType::STRICT})
	        .get_maximal_region_index()
	      == 23);
}

} // namespace
