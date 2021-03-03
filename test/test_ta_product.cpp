/***************************************************************************
 *  test_ta_product.cpp - Test the product of Timed Automata
 *
 *  Created:   Mon  1 Mar 13:58:57 CET 2021
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
#include "automata/ta.h"
#include "automata/ta_product.h"

#include <catch2/catch.hpp>

namespace {

using TA               = automata::ta::TimedAutomaton<std::string, std::string>;
using SingleTransition = automata::ta::Transition<std::string, std::string>;
using ProductTransition =
  automata::ta::Transition<std::tuple<std::string, std::string>, std::string>;
using automata::AtomicClockConstraintT;
using automata::Time;

TEST_CASE("The product of two timed automata", "[ta]")
{
	TA ta1{{"a", "b"}, "1l1", {"1l1"}};
	TA ta2{{"c", "d"}, "2l1", {"2l2"}};
	ta1.add_location("1l2");
	ta1.add_transition(SingleTransition{"1l1", "a", "1l1"});
	ta2.add_transition(SingleTransition{"2l1", "c", "2l2"});
	const auto product = automata::ta::get_product(ta1, ta2);
	CHECK(product.get_alphabet() == std::set<std::string>{"a", "b", "c", "d"});
	CHECK(product.get_initial_location() == std::make_tuple(std::string{"1l1"}, std::string{"2l1"}));
	CHECK(product.get_final_locations()
	      == std::set<std::tuple<std::string, std::string>>{{"1l1", "2l2"}});
	CHECK(product.get_transitions()
	      == std::multimap<std::tuple<std::string, std::string>, ProductTransition>{
	        {{{"1l1", "2l1"}, ProductTransition{{"1l1", "2l1"}, "a", {"1l1", "2l1"}}},
	         {{"1l1", "2l2"}, ProductTransition{{"1l1", "2l2"}, "a", {"1l1", "2l2"}}},
	         {{"1l1", "2l1"}, ProductTransition{{"1l1", "2l1"}, "c", {"1l1", "2l2"}}},
	         {{"1l2", "2l1"}, ProductTransition{{"1l2", "2l1"}, "c", {"1l2", "2l2"}}}}});
	CHECK(product.accepts_word({{"a", 0}, {"c", 1}}));
}

TEST_CASE("The product of two timed automata with clock constraints", "[ta]")
{
	TA ta1{{"a", "b"}, "1l1", {"1l1"}};
	ta1.add_location("1l2");
	ta1.add_clock("c1");
	ta1.add_transition(
	  SingleTransition{"1l1", "a", "1l1", {{"c1", AtomicClockConstraintT<std::less<Time>>{1}}}});
	TA ta2{{"c", "d"}, "2l1", {"2l2"}};
	ta2.add_clock("c2");
	ta2.add_transition(
	  SingleTransition{"2l1", "c", "2l2", {{"c2", AtomicClockConstraintT<std::greater<Time>>{2}}}});
	const auto product = automata::ta::get_product(ta1, ta2);
	CHECK(product.get_alphabet() == std::set<std::string>{"a", "b", "c", "d"});
	CHECK(product.get_initial_location() == std::make_tuple(std::string{"1l1"}, std::string{"2l1"}));
	CHECK(product.get_final_locations()
	      == std::set<std::tuple<std::string, std::string>>{{"1l1", "2l2"}});
	CHECK(product.get_transitions()
	      == std::multimap<std::tuple<std::string, std::string>, ProductTransition>{
	        {{{"1l1", "2l1"},
	          ProductTransition{{"1l1", "2l1"},
	                            "a",
	                            {"1l1", "2l1"},
	                            {{"c1", AtomicClockConstraintT<std::less<Time>>{1}}}}},
	         {{"1l1", "2l2"},
	          ProductTransition{{"1l1", "2l2"},
	                            "a",
	                            {"1l1", "2l2"},
	                            {{"c1", AtomicClockConstraintT<std::less<Time>>{1}}}}},
	         {{"1l1", "2l1"},
	          ProductTransition{{"1l1", "2l1"},
	                            "c",
	                            {"1l1", "2l2"},
	                            {{"c2", AtomicClockConstraintT<std::greater<Time>>{2}}}}},
	         {{"1l2", "2l1"},
	          ProductTransition{{"1l2", "2l1"},
	                            "c",
	                            {"1l2", "2l2"},
	                            {{"c2", AtomicClockConstraintT<std::greater<Time>>{2}}}}}}});
	CHECK(!product.accepts_word({{"a", 0}, {"c", 1}}));
	CHECK(product.accepts_word({{"a", 0}, {"c", 3}}));
	CHECK(!product.accepts_word({{"a", 2}, {"c", 3}}));
}
} // namespace
