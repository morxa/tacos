/***************************************************************************
 *  test_synchronous_product.cpp - Test synchronous products
 *
 *  Created:   Mon 21 Dec 16:56:08 CET 2020
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



#include "automata/ata.h"
#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_regions.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "search/canonical_word.h"
#include "search/operators.h"
#include "search/reg_a.h"
#include "search/search_tree.h"
#include "search/synchronous_product.h"
#include "utilities/Interval.h"
#include "utilities/numbers.h"

#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

namespace {

using namespace tacos;

using automata::ClockSetValuation;
using automata::ClockValuation;
using search::ABRegionSymbol;
using search::ATAConfiguration;
using ATARegionState  = search::ATARegionState<std::string>;
using CanonicalABWord = search::CanonicalABWord<std::string, std::string>;
using search::get_canonical_word;
using search::get_nth_time_successor;
using search::get_time_successor;
using search::get_time_successors;
using search::InvalidCanonicalWordException;
using search::is_valid_canonical_word;
using TARegionState = search::TARegionState<std::string>;
using AP            = logic::AtomicProposition<std::string>;
using Location      = automata::ta::Location<std::string>;

TEST_CASE("Get a canonical word of a simple state", "[canonical_word]")
{
	const logic::MTLFormula                        f{logic::AtomicProposition<std::string>{"a"}};
	const ATAConfiguration<std::string>            ata_configuration = {{f, 0.0}};
	const ClockSetValuation                        v{{"c", 0}};
	const automata::ta::Configuration<std::string> ta_configuration{Location{"s"}, v};
	const auto                                     w =
	  get_canonical_word<std::string, std::string>(ta_configuration, ata_configuration, 5);
	INFO("Canonical word: " << w);
	REQUIRE(w.size() == 1);
	const auto &abs1 = *w.begin();
	REQUIRE(abs1.size() == 2);
	const auto &symbol1 = *abs1.begin();
	REQUIRE(std::holds_alternative<TARegionState>(symbol1));
	CHECK(std::get<TARegionState>(symbol1) == TARegionState{Location{"s"}, "c", 0});
	const auto &symbol2 = *std::next(abs1.begin());
	REQUIRE(std::holds_alternative<ATARegionState>(symbol2));
	CHECK(std::get<ATARegionState>(symbol2) == ATARegionState{f, 0});
}

TEST_CASE("Get a canonical word of a more complex state", "[canonical_word]")
{
	const logic::MTLFormula                        a{logic::AtomicProposition<std::string>{"a"}};
	const logic::MTLFormula                        b{logic::AtomicProposition<std::string>{"b"}};
	const ATAConfiguration<std::string>            ata_configuration = {{a, 0.5}, {b, 1.5}};
	const ClockSetValuation                        v{{"c1", 0.1}, {"c2", 0.5}};
	const automata::ta::Configuration<std::string> ta_configuration{Location{"s"}, v};
	const auto                                     w =
	  get_canonical_word<std::string, std::string>(ta_configuration, ata_configuration, 3);
	INFO("Canonical word: " << w);
	REQUIRE(w.size() == 2);
	{
		const auto &abs1 = *w.begin();
		REQUIRE(abs1.size() == 1);
		const auto &symbol1 = *abs1.begin();
		REQUIRE(std::holds_alternative<TARegionState>(symbol1));
		CHECK(std::get<TARegionState>(symbol1) == TARegionState{Location{"s"}, "c1", 1});
	}
	{
		const auto &abs2 = *std::next(w.begin());
		REQUIRE(abs2.size() == 3);
		const auto &symbol1 = *abs2.begin();
		REQUIRE(std::holds_alternative<TARegionState>(symbol1));
		CHECK(std::get<TARegionState>(symbol1) == TARegionState{Location{"s"}, "c2", 1});
		const auto &symbol2 = *std::next(abs2.begin());
		REQUIRE(std::holds_alternative<ATARegionState>(symbol2));
		CHECK(std::get<ATARegionState>(symbol2) == ATARegionState{a, 1});
		const auto &symbol3 = *std::next(abs2.begin(), 2);
		REQUIRE(std::holds_alternative<ATARegionState>(symbol3));
		CHECK(std::get<ATARegionState>(symbol3) == ATARegionState{b, 3});
	}
}

TEST_CASE("Canonical words with approximately equal fractional parts", "[canonical_word]")
{
	const logic::MTLFormula a{logic::AtomicProposition<std::string>{"a"}};
	CHECK(get_canonical_word(automata::ta::Configuration<std::string>{Location{"l0"},
	                                                                  {{"c1", 0.3}, {"c2", 5.3}}},
	                         ATAConfiguration<std::string>{{a, 10.3}},
	                         11)
	      // All region states should end up in the same partition because they all have the same
	      // fractional part (0.3).
	      == CanonicalABWord({{TARegionState{Location{"l0"}, "c1", 1},
	                           TARegionState{Location{"l0"}, "c2", 11},
	                           ATARegionState{a, 21}}}));
}

TEST_CASE("Cannot get a canonical word if the TA does not have a clock", "[canonical_word]")
{
	CHECK_THROWS_AS(get_canonical_word(automata::ta::Configuration<std::string>{Location{"s"}, {}},
	                                   ATAConfiguration<std::string>{},
	                                   1),
	                std::invalid_argument);
}

TEST_CASE("Validate a canonical word", "[canonical_word]")
{
	using CanonicalABWord = search::CanonicalABWord<std::string, std::string>;
	CHECK_THROWS_AS(is_valid_canonical_word(CanonicalABWord{}), InvalidCanonicalWordException);
	CHECK(is_valid_canonical_word(CanonicalABWord(
	  {{TARegionState{Location{"s0"}, "c0", 0}}, {TARegionState{Location{"s0"}, "c1", 1}}})));
	CHECK_THROWS_AS(is_valid_canonical_word(CanonicalABWord({{}})), InvalidCanonicalWordException);
	CHECK_THROWS_AS(is_valid_canonical_word(
	                  CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0},
	                                    TARegionState{Location{"s0"}, "c1", 1}}})),
	                InvalidCanonicalWordException);
	CHECK_THROWS_AS(is_valid_canonical_word(
	                  CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}},
	                                   {TARegionState{Location{"s0"}, "c1", 0}}})),
	                InvalidCanonicalWordException);
	CHECK_THROWS_AS(is_valid_canonical_word(
	                  CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}},
	                                   {TARegionState{Location{"s0"}, "c1", 2}}})),
	                InvalidCanonicalWordException);
}

TEST_CASE("Comparison of ABRegionSymbols", "[canonical_word]")
{
	using ABRegionSymbol = ABRegionSymbol<std::string, std::string>;
	CHECK(ABRegionSymbol{TARegionState{Location{"l0"}, "x", 0}}
	      < ABRegionSymbol{TARegionState{Location{"l0"}, "x", 1}});
	CHECK(ABRegionSymbol{TARegionState{Location{"l0"}, "x", 0}}
	      < ABRegionSymbol{TARegionState{Location{"l1"}, "x", 0}});
	CHECK(ABRegionSymbol{TARegionState{Location{"l0"}, "x", 1}}
	      < ABRegionSymbol{TARegionState{Location{"l0"}, "y", 0}});
	CHECK(!(ABRegionSymbol{TARegionState{Location{"l0"}, "x", 1}}
	        < ABRegionSymbol{TARegionState{Location{"l0"}, "x", 0}}));
	CHECK(ABRegionSymbol{TARegionState{Location{"l0"}, "x", 0}}
	      == ABRegionSymbol{TARegionState{Location{"l0"}, "x", 0}});
	CHECK(ABRegionSymbol{TARegionState{Location{"l0"}, "x", 0}}
	      < ABRegionSymbol{ATARegionState{AP{"l0"}, 0}});
	CHECK(ABRegionSymbol{TARegionState{Location{"l1"}, "x", 1}}
	      < ABRegionSymbol{ATARegionState{AP{"l0"}, 0}});
	CHECK(ABRegionSymbol{ATARegionState{AP{"s0"}, 0}} < ABRegionSymbol{ATARegionState{AP{"s1"}, 0}});
	CHECK(
	  !(ABRegionSymbol{ATARegionState{AP{"s1"}, 0}} < ABRegionSymbol{ATARegionState{AP{"s0"}, 0}}));
	CHECK(ABRegionSymbol{ATARegionState{AP{"s0"}, 0}} < ABRegionSymbol{ATARegionState{AP{"s0"}, 1}});
	CHECK(
	  !(ABRegionSymbol{ATARegionState{AP{"s0"}, 1}} < ABRegionSymbol{ATARegionState{AP{"s0"}, 0}}));
	CHECK(ABRegionSymbol{ATARegionState{AP{"s0"}, 0}} == ABRegionSymbol{ATARegionState{AP{"s0"}, 0}});
	CHECK(!(ABRegionSymbol{TARegionState{Location{"l0"}, "x", 0}}
	        == ABRegionSymbol{ATARegionState{AP{"l0"}, 0}}));
}

TEST_CASE("Get the time successor for a canonical AB word", "[canonical_word]")
{
	// TODO rewrite test cases to only contain valid words
	using CanonicalABWord = search::CanonicalABWord<std::string, std::string>;
	CHECK(get_time_successor(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}},
	                                          {TARegionState{Location{"s0"}, "c1", 1}}}),
	                         3)
	      == CanonicalABWord(
	        {{TARegionState{Location{"s0"}, "c0", 1}}, {TARegionState{Location{"s0"}, "c1", 1}}}));
	CHECK(get_time_successor(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}), 3)
	      == CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 1}}}));
	CHECK(get_time_successor(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}},
	                                          {TARegionState{Location{"s0"}, "c1", 1}},
	                                          {TARegionState{Location{"s0"}, "c2", 3}}}),
	                         3)
	      == CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 1}},
	                          {TARegionState{Location{"s0"}, "c1", 1}},
	                          {TARegionState{Location{"s0"}, "c2", 3}}}));
	CHECK(get_time_successor(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 1}},
	                                          {TARegionState{Location{"s0"}, "c1", 3}}}),
	                         3)
	      == CanonicalABWord(
	        {{TARegionState{Location{"s0"}, "c1", 4}}, {TARegionState{Location{"s0"}, "c0", 1}}}));
	CHECK(get_time_successor(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 1}},
	                                          {TARegionState{Location{"s0"}, "c1", 1}}}),
	                         3)
	      == CanonicalABWord(
	        {{TARegionState{Location{"s0"}, "c1", 2}}, {TARegionState{Location{"s0"}, "c0", 1}}}));
	const logic::AtomicProposition<std::string> a{"a"};
	const logic::AtomicProposition<std::string> b{"b"};
	CHECK(get_time_successor(CanonicalABWord({{ATARegionState{a, 0}},
	                                          {ATARegionState{b, 1}},
	                                          {ATARegionState{a || b, 3}}}),
	                         3)
	      == CanonicalABWord(
	        {{ATARegionState{a, 1}}, {ATARegionState{b, 1}}, {ATARegionState{a || b, 3}}}));
	CHECK(get_time_successor(CanonicalABWord({{ATARegionState{a, 7}}}), 3)
	      == CanonicalABWord({{ATARegionState{a, 7}}}));
	CHECK(get_time_successor(CanonicalABWord({{ATARegionState{b, 3}}, {ATARegionState{a, 7}}}), 3)
	      == CanonicalABWord({{ATARegionState{b, 4}}, {ATARegionState{a, 7}}}));

	CHECK(get_time_successor(CanonicalABWord({{ATARegionState{b, 3}, ATARegionState{a, 7}}}), 3)
	      == CanonicalABWord({{ATARegionState{b, 4}}, {ATARegionState{a, 7}}}));

	CHECK(get_time_successor(CanonicalABWord({{TARegionState{Location{"s1"}, "c0", 4}},
	                                          {TARegionState{Location{"s0"}, "c0", 3}},
	                                          {ATARegionState{a, 7}}}),
	                         3)
	      == CanonicalABWord({{TARegionState{Location{"s1"}, "c0", 5}},
	                          {TARegionState{Location{"s0"}, "c0", 3}},
	                          {ATARegionState{a, 7}}}));
	CHECK(get_time_successor(CanonicalABWord({{ATARegionState{b, 1}, ATARegionState{a, 3}}}), 3)
	      == CanonicalABWord({{ATARegionState{b, 2}, ATARegionState{a, 4}}}));
	CHECK(get_time_successor(
	        CanonicalABWord({{TARegionState{Location{"l0"}, "x", 1}, ATARegionState{a, 5}}}), 2)
	      == CanonicalABWord({{TARegionState{Location{"l0"}, "x", 2}}, {ATARegionState{a, 5}}}));

	CHECK(get_time_successor(CanonicalABWord{{TARegionState{Location{"l0"}, "x0", 0}},
	                                         {TARegionState{Location{"l0"}, "x1", 1}},
	                                         {TARegionState{Location{"l0"}, "x3", 3}}},
	                         1)
	      == CanonicalABWord{{TARegionState{Location{"l0"}, "x0", 1}},
	                         {TARegionState{Location{"l0"}, "x1", 1}},
	                         {TARegionState{Location{"l0"}, "x3", 3}}});
	// x2 is incremented and should end up in the last partition with the maxed regions.
	CHECK(get_time_successor(CanonicalABWord{{TARegionState{Location{"l0"}, "x2", 2}},
	                                         {TARegionState{Location{"l0"}, "x3", 3}}},
	                         1)
	      == CanonicalABWord{
	        {TARegionState{Location{"l0"}, "x2", 3}, TARegionState{Location{"l0"}, "x3", 3}}});
	CHECK(get_time_successor(CanonicalABWord{{TARegionState{Location{"l0"}, "x0", 0},
	                                          TARegionState{Location{"l0"}, "x2", 2}},
	                                         {TARegionState{Location{"l0"}, "x1", 1}},
	                                         {TARegionState{Location{"l0"}, "x3", 3}}},
	                         1)
	      == CanonicalABWord{{TARegionState{Location{"l0"}, "x0", 1}},
	                         {TARegionState{Location{"l0"}, "x1", 1}},
	                         {TARegionState{Location{"l0"}, "x2", 3},
	                          TARegionState{Location{"l0"}, "x3", 3}}});

	// Both x0 and x2 are incremented and should be split. x2 should end up in the maxed partition
	// with x3.
	CHECK(get_time_successor(CanonicalABWord{{TARegionState{Location{"l0"}, "x0", 0},
	                                          TARegionState{Location{"l0"}, "x2", 2}},
	                                         {TARegionState{Location{"l0"}, "x3", 3}}},
	                         1)
	      == CanonicalABWord{{TARegionState{Location{"l0"}, "x0", 1}},
	                         {TARegionState{Location{"l0"}, "x2", 3},
	                          TARegionState{Location{"l0"}, "x3", 3}}});

	// Successor of successor.
	CHECK(get_time_successor(
	        get_time_successor(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}), 3), 3)
	      == CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 2}}}));
	CHECK(search::get_nth_time_successor(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}),
	                                     2,
	                                     3)
	      == CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 2}}}));
	CHECK(search::get_nth_time_successor(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}),
	                                     0,
	                                     3)
	      == CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}));
	CHECK(search::get_nth_time_successor(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}),
	                                     7,
	                                     3)
	      == search::get_nth_time_successor(
	        CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}), 8, 3));
	CHECK(get_time_successor(CanonicalABWord{{ATARegionState{a, 0}},
	                                         {TARegionState{Location{"s0"}, "c0", 1}}},
	                         1)
	      == CanonicalABWord{{ATARegionState{a, 1}}, {TARegionState{Location{"s0"}, "c0", 1}}});
	CHECK(get_time_successor(CanonicalABWord{{ATARegionState{a, 1}},
	                                         {TARegionState{Location{"s0"}, "c0", 1}}},
	                         1)
	      == CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 2}}, {ATARegionState{a, 1}}});
}

TEST_CASE("Compute the time successors of a set of nodes", "[canonical_word]")
{
	const logic::AtomicProposition<std::string> a{"a"};
	const logic::AtomicProposition<std::string> b{"b"};
	const logic::AtomicProposition<std::string> c{"c"};

	const CanonicalABWord w1{{TARegionState{Location{"s0"}, "c0", 1}}};
	const CanonicalABWord w2{{ATARegionState{a, 0}}, {TARegionState{Location{"s0"}, "c0", 1}}};
	const CanonicalABWord w3{{ATARegionState{b, 1}, TARegionState{Location{"s0"}, "c0", 1}}};
	const CanonicalABWord w4{{TARegionState{Location{"s0"}, "c0", 1}}, {ATARegionState{c, 1}}};
	const CanonicalABWord w5{{TARegionState{Location{"s0"}, "c0", 1}}, {ATARegionState{a, 1}}};
	SECTION("first pair")
	{
		const auto words      = std::set{{w1, w2}};
		const auto successors = get_time_successors(words, 1);
		CAPTURE(successors);
		CHECK(successors.size() == 6);
		REQUIRE(successors.size() >= 6);
		CHECK(successors[0] == words);
		CHECK(successors[1]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 1}}},
		        CanonicalABWord{{ATARegionState{a, 1}}, {TARegionState{Location{"s0"}, "c0", 1}}},
		      });
		CHECK(successors[2]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 2}}},
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 2}}, {ATARegionState{a, 1}}},
		      });
		CHECK(successors[3]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 3}}},
		        CanonicalABWord{{ATARegionState{a, 1}}, {TARegionState{Location{"s0"}, "c0", 3}}},
		      });
		CHECK(successors[4]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 3}}},
		        CanonicalABWord{{ATARegionState{a, 2}}, {TARegionState{Location{"s0"}, "c0", 3}}},
		      });
		CHECK(successors[5]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 3}}},
		        CanonicalABWord{{ATARegionState{a, 3}, TARegionState{Location{"s0"}, "c0", 3}}},
		      });
	}
	SECTION("second pair")
	{
		const auto words      = std::set{{w1, w3}};
		const auto successors = get_time_successors(words, 1);
		CAPTURE(successors);
		CHECK(successors.size() == 3);
		REQUIRE(successors.size() >= 3);
		CHECK(successors[0] == words);
		CHECK(successors[1]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 2}}},
		        CanonicalABWord{{ATARegionState{b, 2}, TARegionState{Location{"s0"}, "c0", 2}}},
		      });
		CHECK(successors[2]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 3}}},
		        CanonicalABWord{{ATARegionState{b, 3}, TARegionState{Location{"s0"}, "c0", 3}}},
		      });
	}
	SECTION("third pair")
	{
		const auto words      = std::set{{w2, w3}};
		const auto successors = get_time_successors(words, 1);
		CAPTURE(successors);
		CHECK(successors.size() == 6);
		REQUIRE(successors.size() >= 6);
		CHECK(successors[0] == words);
		CHECK(successors[1]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{ATARegionState{a, 1}}, {TARegionState{Location{"s0"}, "c0", 1}}},
		        CanonicalABWord{{ATARegionState{b, 1}, TARegionState{Location{"s0"}, "c0", 1}}},
		      });
		CHECK(successors[2]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 2}}, {ATARegionState{a, 1}}},
		        CanonicalABWord{{ATARegionState{b, 2}, TARegionState{Location{"s0"}, "c0", 2}}},
		      });
		CHECK(successors[3]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{ATARegionState{a, 1}}, {TARegionState{Location{"s0"}, "c0", 3}}},
		        CanonicalABWord{{ATARegionState{b, 3}, TARegionState{Location{"s0"}, "c0", 3}}},
		      });
		CHECK(successors[4]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{ATARegionState{a, 2}}, {TARegionState{Location{"s0"}, "c0", 3}}},
		        CanonicalABWord{{ATARegionState{b, 3}, TARegionState{Location{"s0"}, "c0", 3}}},
		      });
		CHECK(successors[5]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{ATARegionState{a, 3}, TARegionState{Location{"s0"}, "c0", 3}}},
		        CanonicalABWord{{ATARegionState{b, 3}, TARegionState{Location{"s0"}, "c0", 3}}},
		      });
	}
	SECTION("fourth pair")
	{
		const auto words      = std::set{{w3, w4}};
		const auto successors = get_time_successors(words, 1);
		CAPTURE(successors);
		CHECK(successors.size() == 5);
		REQUIRE(successors.size() >= 5);
		CHECK(successors[0] == words);
		CHECK(successors[1]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{ATARegionState{b, 1}, TARegionState{Location{"s0"}, "c0", 1}}},
		        CanonicalABWord{{ATARegionState{c, 2}}, {TARegionState{Location{"s0"}, "c0", 1}}},
		      });
		CHECK(successors[2]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{ATARegionState{b, 1}, TARegionState{Location{"s0"}, "c0", 1}}},
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 1}}, {ATARegionState{c, 3}}},
		      });
		CHECK(successors[3]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{ATARegionState{b, 2}, TARegionState{Location{"s0"}, "c0", 2}}},
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 2}}, {ATARegionState{c, 3}}},
		      });
		CHECK(successors[4]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{ATARegionState{b, 3}, TARegionState{Location{"s0"}, "c0", 3}}},
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 3}, ATARegionState{c, 3}}},
		      });
	}
	SECTION("fifth pair")
	{
		// We assume that both clocks have the same value, because they are in the same region and both
		// have the same position relative to the TA states. However, this assumption is not necessarily
		// correct!
		const auto words      = std::set{{w4, w5}};
		const auto successors = get_time_successors(words, 1);
		CAPTURE(successors);
		CHECK(successors.size() == 5);
		REQUIRE(successors.size() >= 5);
		CHECK(successors[0] == words);
		CHECK(successors[1]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{ATARegionState{c, 2}}, {TARegionState{Location{"s0"}, "c0", 1}}},
		        CanonicalABWord{{ATARegionState{a, 2}}, {TARegionState{Location{"s0"}, "c0", 1}}},
		      });
		CHECK(successors[2]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 1}}, {ATARegionState{c, 3}}},
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 1}}, {ATARegionState{a, 3}}},
		      });
		CHECK(successors[3]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 2}}, {ATARegionState{c, 3}}},
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 2}}, {ATARegionState{a, 3}}},
		      });
		CHECK(successors[4]
		      == std::set<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 3}, ATARegionState{c, 3}}},
		        CanonicalABWord{{TARegionState{Location{"s0"}, "c0", 3}, ATARegionState{a, 3}}},
		      });
	}
}

TEST_CASE("Get a concrete candidate for a canonical word", "[canonical_word]")
{
	using automata::Time;
	using automata::ta::Integer;
	using TAConf    = search::TAConfiguration<std::string>;
	using ATAConf   = search::ATAConfiguration<std::string>;
	using Candidate = std::pair<TAConf, ATAConf>;
	using search::get_candidate;
	using utilities::getFractionalPart;
	using utilities::getIntegerPart;
	const logic::AtomicProposition<std::string> a{"a"};
	const logic::AtomicProposition<std::string> b{"b"};

	// single state with fractional part 0, clock 0
	CHECK(get_candidate(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}))
	      == Candidate(TAConf{Location{"s0"}, {{"c0", 0}}}, ATAConf{}));
	// single state with fractional part 0, clock != 0
	CHECK(get_candidate(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 2}}}))
	      == Candidate(TAConf{Location{"s0"}, {{"c0", 1}}}, ATAConf{}));

	{
		// single state with non-zero fractional part in (0, 1)
		const Candidate cand =
		  get_candidate(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 1}}}));
		CHECK(cand.first.clock_valuations.at("c0") > 0.0);
		CHECK(cand.first.clock_valuations.at("c0") < 1.0);
		// The ATA configuration must be empty.
		CHECK(cand.second.empty());
	}

	{
		// single state with non-zero fractional part not in (0, 1)
		const Candidate cand =
		  get_candidate(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 5}}}));
		CHECK(cand.first.clock_valuations.at("c0") > 2.0);
		CHECK(cand.first.clock_valuations.at("c0") < 3.0);
		// The ATA configuration must be empty.
		CHECK(cand.second.empty());
	}

	// A single ATA state
	CHECK(get_candidate(CanonicalABWord({{ATARegionState{a, 0}}})) == Candidate({}, {{a, 0}}));
	CHECK(get_candidate(CanonicalABWord({{ATARegionState{a, 2}}})) == Candidate({}, {{a, 1}}));
	{
		// A single ATA state with fractional part in (0, 1)
		const Candidate cand = get_candidate(CanonicalABWord({{ATARegionState{a, 1}}}));
		REQUIRE(cand.second.size() == 1);
		const auto v = cand.second.begin()->clock_valuation;
		CHECK(getFractionalPart<Integer>(v) > 0);
		CHECK(getIntegerPart<Integer>(v) == 0);
	}

	{
		// A single ATA state with fractional part not in (0, 1)
		const Candidate cand = get_candidate(CanonicalABWord({{ATARegionState{a, 3}}}));
		REQUIRE(cand.second.size() == 1);
		const auto v = cand.second.begin()->clock_valuation;
		CHECK(getFractionalPart<Integer>(v) > 0);
		CHECK(getIntegerPart<Integer>(v) == 1);
	}

	{
		// two clocks, both non-fractional with same integer parts
		const Candidate cand = get_candidate(CanonicalABWord(
		  {{TARegionState{Location{"s0"}, "c0", 2}, TARegionState{Location{"s0"}, "c1", 2}}}));
		CHECK(getFractionalPart<Integer>(cand.first.clock_valuations.at("c0")) == 0);
		CHECK(getFractionalPart<Integer>(cand.first.clock_valuations.at("c1")) == 0);
		CHECK(getIntegerPart<Integer>(cand.first.clock_valuations.at("c0"))
		      == getIntegerPart<Integer>(cand.first.clock_valuations.at("c1")));
		// The ATA configuration must be empty.
		CHECK(cand.second.empty());
	}

	{
		// two clocks, both non-fractional but with different integer parts
		const Candidate cand = get_candidate(CanonicalABWord(
		  {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 2}}}));
		CHECK(getFractionalPart<Integer>(cand.first.clock_valuations.at("c0")) == 0);
		CHECK(getFractionalPart<Integer>(cand.first.clock_valuations.at("c1")) == 0);
		CHECK(getIntegerPart<Integer>(cand.first.clock_valuations.at("c0"))
		      < getIntegerPart<Integer>(cand.first.clock_valuations.at("c1")));
		// The ATA configuration must be empty.
		CHECK(cand.second.empty());
	}

	{
		// two states, one with a clock with fractional part, the other one without
		const Candidate cand = get_candidate(CanonicalABWord(
		  {{TARegionState{Location{"s0"}, "c0", 2}}, {TARegionState{Location{"s0"}, "c1", 1}}}));
		CHECK(cand.first.clock_valuations.at("c0") == 1.0);
		CHECK(cand.first.clock_valuations.at("c1") > 0.0);
		CHECK(cand.first.clock_valuations.at("c1") < 1.0);
		// The ATA configuration must be empty.
		CHECK(cand.second.empty());
	}

	{
		// two states, both clocks fractional with equal fractional parts and equal integer
		// parts
		const Candidate cand = get_candidate(CanonicalABWord(
		  {{TARegionState{Location{"s0"}, "c0", 1}, TARegionState{Location{"s0"}, "c1", 1}}}));
		CHECK(cand.first.clock_valuations.at("c0") == cand.first.clock_valuations.at("c1"));
	}

	{
		// two states, both clocks fractional with equal fractional parts but different integer
		// parts
		const Candidate cand = get_candidate(CanonicalABWord(
		  {{TARegionState{Location{"s0"}, "c0", 1}, TARegionState{Location{"s0"}, "c1", 3}}}));
		CHECK(getFractionalPart<Integer>(cand.first.clock_valuations.at("c0"))
		      == getFractionalPart<Integer>(cand.first.clock_valuations.at("c1")));
		CHECK(getIntegerPart<Integer>(cand.first.clock_valuations.at("c0"))
		      < getIntegerPart<Integer>(cand.first.clock_valuations.at("c1")));
	}

	{
		// two states, both clocks fractional with different fractional parts but same integer
		// parts
		const Candidate cand = get_candidate(CanonicalABWord(
		  {{TARegionState{Location{"s0"}, "c0", 1}}, {TARegionState{Location{"s0"}, "c1", 1}}}));
		CHECK(cand.first.clock_valuations.at("c0") < cand.first.clock_valuations.at("c1"));
		CHECK(getFractionalPart<Integer>(cand.first.clock_valuations.at("c0"))
		      < getFractionalPart<Integer>(cand.first.clock_valuations.at("c1")));
		CHECK(getIntegerPart<Integer>(cand.first.clock_valuations.at("c0"))
		      == getIntegerPart<Integer>(cand.first.clock_valuations.at("c1")));
	}

	{
		// two states, both clocks fractional with different fractional and integer parts
		const Candidate cand = get_candidate(CanonicalABWord(
		  {{TARegionState{Location{"s0"}, "c0", 1}}, {TARegionState{Location{"s0"}, "c1", 3}}}));
		CHECK(cand.first.clock_valuations.at("c0") < cand.first.clock_valuations.at("c1"));
		CHECK(getFractionalPart<Integer>(cand.first.clock_valuations.at("c0"))
		      < getFractionalPart<Integer>(cand.first.clock_valuations.at("c1")));
		CHECK(getIntegerPart<Integer>(cand.first.clock_valuations.at("c0"))
		      < getIntegerPart<Integer>(cand.first.clock_valuations.at("c1")));
	}

	{
		// several clocks with different regions
		const Candidate cand = get_candidate(CanonicalABWord(
		  {{TARegionState{Location{"s0"}, "c0", 0}},
		   {TARegionState{Location{"s0"}, "c1", 1}, TARegionState{Location{"s0"}, "c2", 3}},
		   {TARegionState{Location{"s0"}, "c3", 1}}}));
		CHECK(cand.first.clock_valuations.at("c0") == 0.0);
		CHECK(cand.first.clock_valuations.at("c1") > 0.0);
		CHECK(cand.first.clock_valuations.at("c2") > 1.0);
		CHECK(cand.first.clock_valuations.at("c3") > 0.0);
		CHECK(cand.first.clock_valuations.at("c1") < 1.0);
		CHECK(cand.first.clock_valuations.at("c2") < 2.0);
		CHECK(cand.first.clock_valuations.at("c3") < 1.0);
		CHECK(cand.first.clock_valuations.at("c1") == cand.first.clock_valuations.at("c2") - 1.0);
		CHECK(cand.first.clock_valuations.at("c1") < cand.first.clock_valuations.at("c3"));
	}
}

TEST_CASE("Get the next canonical word(s)", "[canonical_word]")
{
	using TATransition     = automata::ta::Transition<std::string, std::string>;
	using TA               = automata::ta::TimedAutomaton<std::string, std::string>;
	using TAConfiguration  = automata::ta::Configuration<std::string>;
	using ATAConfiguration = automata::ata::Configuration<logic::MTLFormula<std::string>>;
	using automata::AtomicClockConstraintT;
	using utilities::arithmetic::BoundType;
	TA ta{{Location{"s0"}, Location{"s1"}, Location{"s2"}},
	      {"a", "b", "c"},
	      Location{"s0"},
	      {Location{"s0"}, Location{"s1"}, Location{"s2"}},
	      {"x"},
	      {TATransition(Location{"s0"},
	                    "a",
	                    Location{"s0"},
	                    {{"x", AtomicClockConstraintT<std::greater<automata::Time>>(1)}},
	                    {"x"}),
	       TATransition(Location{"s0"},
	                    "b",
	                    Location{"s1"},
	                    {{"x", AtomicClockConstraintT<std::less<automata::Time>>(1)}}),
	       TATransition(Location{"s0"}, "c", Location{"s2"}),
	       TATransition(Location{"s2"}, "b", Location{"s1"})}};

	SECTION("with action constraints")
	{
		logic::MTLFormula<std::string> a{AP("a")};
		logic::MTLFormula<std::string> b{AP("b")};

		logic::MTLFormula f   = a.until(b);
		auto              ata = mtl_ata_translation::translate(f);

		auto initial_word = get_canonical_word(TAConfiguration{Location{"s0"}, {{"x", 0}}},
		                                       ata.get_initial_configuration(),
		                                       2);
		CHECK(initial_word
		      == CanonicalABWord({{TARegionState{Location{"s0"}, "x", 0},
		                           ATARegionState{logic::MTLFormula{AP{"l0"}}, 0}}}));
		CHECK(search::get_next_canonical_words(ta,
		                                       ata,
		                                       {ta.get_initial_configuration(),
		                                        ata.get_initial_configuration()},
		                                       std::string{"b"},
		                                       2)
		      == std::vector<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s1"}, "x", 0}, ATARegionState{f, 0}}}});
		CHECK(
		  search::get_next_canonical_words(
		    ta, ata, {ta.get_initial_configuration(), ATAConfiguration{{f, 0}}}, std::string{"b"}, 2)
		  == std::vector<CanonicalABWord>{CanonicalABWord{{TARegionState{Location{"s1"}, "x", 0}}}});
	}
	SECTION("with location constraints")
	{
		logic::MTLFormula<std::string> s0{AP("s0")};
		logic::MTLFormula<std::string> s1{AP("s1")};
		auto                           f   = s0.until(s1);
		auto                           ata = mtl_ata_translation::translate(f);
		CHECK(search::get_next_canonical_words<std::string, std::string, std::string, true>(
		        ta, ata, {ta.get_initial_configuration(), ata.get_initial_configuration()}, "b", 2)
		      == std::vector<CanonicalABWord>{
		        CanonicalABWord{{TARegionState{Location{"s1"}, "x", 0}, ATARegionState{f, 0}}}});
		CHECK(
		  search::get_next_canonical_words<std::string, std::string, std::string, true>(
		    ta, ata, {TAConfiguration{Location{"s0"}, {{"x", 0}}}, ATAConfiguration{{{f, 0}}}}, "b", 2)
		  == std::vector<CanonicalABWord>{CanonicalABWord{{TARegionState{Location{"s1"}, "x", 0}}}});
	}
}

TEST_CASE("reg_a", "[canonical_word]")
{
	CHECK(search::reg_a(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}))
	      == CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}));
	CHECK(search::reg_a(CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0},
	                                      ATARegionState{logic::MTLFormula{AP{"a"}}, 0}}}))
	      == CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}));
	CHECK(search::reg_a(CanonicalABWord({{TARegionState{Location{"s1"}, "c0", 0}},
	                                     {ATARegionState{logic::MTLFormula{AP{"b"}}, 3}}}))
	      == CanonicalABWord({{TARegionState{Location{"s1"}, "c0", 0}}}));
}

TEST_CASE("monotone_domination_order", "[canonical_word]")
{
	CHECK(search::is_monotonically_dominated(
	  CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}),
	  CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}})));
	CHECK(!search::is_monotonically_dominated(
	  CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}),
	  CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 1}}})));
	CHECK(!search::is_monotonically_dominated(
	  CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, ATARegionState{logic::MTLFormula{AP{"a"}}, 0}}}),
	  CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}})));
	CHECK(search::is_monotonically_dominated(
	  CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 0}}}),
	  CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 0}}})));
	CHECK(!search::is_monotonically_dominated(
	  CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 1}},
	     {ATARegionState{logic::MTLFormula{AP{"a"}}, 0}}}),
	  CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 1}}})));
	CHECK(search::is_monotonically_dominated(
	  CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 1}}}),
	  CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 1}},
	     {ATARegionState{logic::MTLFormula{AP{"a"}}, 0}}})));
	CHECK(search::is_monotonically_dominated(
	  CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}}),
	  CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 1}},
	     {ATARegionState{logic::MTLFormula{AP{"a"}}, 0}}})));
}

TEST_CASE("monotone_domination_order_sets", "[canonical_word]")
{
	CHECK(
	  search::is_monotonically_dominated(std::set<CanonicalABWord>{}, std::set<CanonicalABWord>{}));

	CHECK(search::is_monotonically_dominated(
	  std::set<CanonicalABWord>{CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 1}}})},
	  std::set<CanonicalABWord>{CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 1}}})}));

	CHECK(search::is_monotonically_dominated(std::set<CanonicalABWord>{CanonicalABWord(
	                                           {{TARegionState{Location{"s0"}, "c0", 0},
	                                             TARegionState{Location{"s0"}, "c1", 1}}})},
	                                         std::set<CanonicalABWord>{}));

	CHECK(!search::is_monotonically_dominated(std::set<CanonicalABWord>{},
	                                          std::set<CanonicalABWord>{CanonicalABWord(
	                                            {{TARegionState{Location{"s0"}, "c0", 0},
	                                              TARegionState{Location{"s0"}, "c1", 1}}})}));

	CHECK(!search::is_monotonically_dominated(
	  std::set<CanonicalABWord>{CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}})},
	  std::set<CanonicalABWord>{CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 2}}})}));

	CHECK(search::is_monotonically_dominated(
	  std::set<CanonicalABWord>{CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0},
	                                              TARegionState{Location{"s0"}, "c1", 1}},
	                                             {ATARegionState{logic::MTLFormula{AP{"a"}}, 0}}}),
	                            CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0},
	                                              TARegionState{Location{"s0"}, "c1", 1}},
	                                             {ATARegionState{logic::MTLFormula{AP{"a"}}, 1}}})},
	  std::set<CanonicalABWord>{CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 1}},
	     {ATARegionState{logic::MTLFormula{AP{"a"}}, 0}}})}));

	CHECK(!search::is_monotonically_dominated(
	  std::set<CanonicalABWord>{CanonicalABWord(
	    {{TARegionState{Location{"s0"}, "c0", 0}, TARegionState{Location{"s0"}, "c1", 1}},
	     {ATARegionState{logic::MTLFormula{AP{"a"}}, 0}}})},
	  std::set<CanonicalABWord>{CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0},
	                                              TARegionState{Location{"s0"}, "c1", 1}},
	                                             {ATARegionState{logic::MTLFormula{AP{"a"}}, 0}}}),
	                            CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0},
	                                              TARegionState{Location{"s0"}, "c1", 1}},
	                                             {ATARegionState{logic::MTLFormula{AP{"a"}}, 1}}})}));
}

TEST_CASE("Monotonic domination on nodes", "[canonical_word]")
{
	using Node = search::SearchTreeNode<std::string, std::string, std::string>;
	auto n1 =
	  std::make_shared<Node>(std::set{CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 0}}})});
	auto n2 =
	  std::make_shared<Node>(std::set{CanonicalABWord({{TARegionState{Location{"s1"}, "c0", 0}}})});
	auto n3 = std::make_shared<Node>(std::set{CanonicalABWord(
	  {{TARegionState{Location{"s0"}, "c0", 0}, ATARegionState{logic::MTLFormula{AP{"a"}}, 1}}})});
	SECTION("Self domination")
	{
		n1->add_child({0, "a"}, n2);
		n2->add_child({0, "a"}, n1);
		// n1 mon.doms itself, but this is explicitly ignored.
		CHECK(!search::dominates_ancestor(n1.get()));
	}
	SECTION("Ancestor domination")
	{
		n1->add_child({0, "a"}, n2);
		n2->add_child({0, "a"}, n3);
		CHECK(search::dominates_ancestor(n3.get()));
	}
	SECTION("Domination via the right parent")
	{
		/*
		    n1
		   /  \
		   n2 n3
		   |  |
		   n4 n5
		   \  /
		    n6
		 where n6 dominates n2 and no other node
		*/
		auto n1 =
		  std::make_shared<Node>(std::set{CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 1}}})});
		auto n2 =
		  std::make_shared<Node>(std::set{CanonicalABWord({{TARegionState{Location{"s1"}, "c0", 0}}})});
		auto n3 =
		  std::make_shared<Node>(std::set{CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 3}}})});
		auto n4 =
		  std::make_shared<Node>(std::set{CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 4}}})});
		auto n5 =
		  std::make_shared<Node>(std::set{CanonicalABWord({{TARegionState{Location{"s0"}, "c0", 5}}})});
		auto n6 = std::make_shared<Node>(std::set{CanonicalABWord(
		  {{TARegionState{Location{"s1"}, "c0", 0}, ATARegionState{logic::MTLFormula{AP{"a"}}, 1}}})});
		n1->add_child({0, "a"}, n2);
		n1->add_child({0, "b"}, n3);
		n2->add_child({0, "a"}, n4);
		n3->add_child({0, "a"}, n5);
		n5->add_child({0, "a"}, n6);
		// No domination yet, as the link n4->n6 is missing.
		CHECK(!search::dominates_ancestor(n6.get()));
		n4->add_child({0, "a"}, n6);
		// Now, n6 is dominating via n4->n2.
		CHECK(search::dominates_ancestor(n6.get()));
		// All the other nodes are not dominating.
		CHECK(!search::dominates_ancestor(n1.get()));
		CHECK(!search::dominates_ancestor(n2.get()));
		CHECK(!search::dominates_ancestor(n3.get()));
		CHECK(!search::dominates_ancestor(n4.get()));
		CHECK(!search::dominates_ancestor(n5.get()));
	}
}
} // namespace
