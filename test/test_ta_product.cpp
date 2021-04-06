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

#include <catch2/catch_test_macros.hpp>

namespace {

using TA                = automata::ta::TimedAutomaton<std::string, std::string>;
using SingleTransition  = automata::ta::Transition<std::string, std::string>;
using ProductTransition = automata::ta::Transition<std::vector<std::string>, std::string>;
using SingleLocation    = automata::ta::Location<std::string>;
using ProductLocation   = automata::ta::Location<std::vector<std::string>>;
using automata::AtomicClockConstraintT;
using automata::Time;
using automata::ta::get_product;

TEST_CASE("The product of two timed automata", "[ta]")
{
	TA ta1{{"a", "b"}, SingleLocation{"1l1"}, {SingleLocation{"1l1"}}};
	TA ta2{{"c", "d"}, SingleLocation{"2l1"}, {SingleLocation{"2l2"}}};
	ta1.add_location(SingleLocation{"1l2"});
	ta1.add_transition(SingleTransition{SingleLocation{"1l1"}, "a", SingleLocation{"1l1"}});
	ta2.add_transition(SingleTransition{SingleLocation{"2l1"}, "c", SingleLocation{"2l2"}});
	const auto product = get_product<std::string, std::string>({ta1, ta2});
	CHECK(product.get_alphabet() == std::set<std::string>{"a", "b", "c", "d"});
	CHECK(product.get_initial_location() == ProductLocation{{"1l1", "2l1"}});
	CHECK(product.get_final_locations() == std::set{ProductLocation{{"1l1", "2l2"}}});
	CHECK(
	  product.get_transitions()
	  == std::multimap<ProductLocation, ProductTransition>{
	    {{ProductLocation{{"1l1", "2l1"}},
	      ProductTransition{ProductLocation{{"1l1", "2l1"}}, "a", ProductLocation{{"1l1", "2l1"}}}},
	     {ProductLocation{{"1l1", "2l2"}},
	      ProductTransition{ProductLocation{{"1l1", "2l2"}}, "a", ProductLocation{{"1l1", "2l2"}}}},
	     {ProductLocation{{"1l1", "2l1"}},
	      ProductTransition{ProductLocation{{"1l1", "2l1"}}, "c", ProductLocation{{"1l1", "2l2"}}}},
	     {ProductLocation{{"1l2", "2l1"}},
	      ProductTransition{
	        ProductLocation{{"1l2", "2l1"}}, "c", ProductLocation{{"1l2", "2l2"}}}}}});
	CHECK(product.accepts_word({{"a", 0}, {"c", 1}}));

	// Synchronized actions are not implemented.
	CHECK_THROWS_AS((get_product<std::string, std::string>({ta1, ta2}, {"a"})),
	                automata::ta::NotImplementedException);
}

TEST_CASE("The product of two timed automata with clock constraints", "[ta]")
{
	TA ta1{{"a", "b"}, SingleLocation{"1l1"}, {SingleLocation{"1l1"}}};
	ta1.add_location(SingleLocation{"1l2"});
	ta1.add_clock("c1");
	ta1.add_transition(SingleTransition{SingleLocation{"1l1"},
	                                    "a",
	                                    SingleLocation{"1l1"},
	                                    {{"c1", AtomicClockConstraintT<std::less<Time>>{1}}}});
	TA ta2{{"c", "d"}, SingleLocation{"2l1"}, {SingleLocation{"2l2"}}};
	ta2.add_clock("c2");
	ta2.add_transition(SingleTransition{SingleLocation{"2l1"},
	                                    "c",
	                                    SingleLocation{"2l2"},
	                                    {{"c2", AtomicClockConstraintT<std::greater<Time>>{2}}}});
	const auto product = get_product<std::string, std::string>({ta1, ta2});
	CHECK(product.get_alphabet() == std::set<std::string>{"a", "b", "c", "d"});
	CHECK(product.get_initial_location() == ProductLocation{{"1l1", "2l1"}});
	CHECK(product.get_final_locations() == std::set{ProductLocation{{"1l1", "2l2"}}});
	CHECK(product.get_transitions()
	      == std::multimap<ProductLocation, ProductTransition>{
	        {{ProductLocation{{"1l1", "2l1"}},
	          ProductTransition{ProductLocation{{"1l1", "2l1"}},
	                            "a",
	                            ProductLocation{{"1l1", "2l1"}},
	                            {{"c1", AtomicClockConstraintT<std::less<Time>>{1}}}}},
	         {ProductLocation{{"1l1", "2l2"}},
	          ProductTransition{ProductLocation{{"1l1", "2l2"}},
	                            "a",
	                            ProductLocation{{"1l1", "2l2"}},
	                            {{"c1", AtomicClockConstraintT<std::less<Time>>{1}}}}},
	         {ProductLocation{{"1l1", "2l1"}},
	          ProductTransition{ProductLocation{{"1l1", "2l1"}},
	                            "c",
	                            ProductLocation{{"1l1", "2l2"}},
	                            {{"c2", AtomicClockConstraintT<std::greater<Time>>{2}}}}},
	         {ProductLocation{{"1l2", "2l1"}},
	          ProductTransition{ProductLocation{{"1l2", "2l1"}},
	                            "c",
	                            ProductLocation{{"1l2", "2l2"}},
	                            {{"c2", AtomicClockConstraintT<std::greater<Time>>{2}}}}}}});
	CHECK(!product.accepts_word({{"a", 0}, {"c", 1}}));
	CHECK(product.accepts_word({{"a", 0}, {"c", 3}}));
	CHECK(!product.accepts_word({{"a", 2}, {"c", 3}}));
}

TEST_CASE("The product of three timed automata", "[ta]")
{
	TA   ta1{{SingleLocation{"1l0"}, SingleLocation{"1l1"}},
         {"1a", "1b"},
         SingleLocation{"1l0"},
         {SingleLocation{"1l1"}},
         {"1c1"},
         {SingleTransition{SingleLocation{"1l0"},
                           "1a",
                           SingleLocation{"1l1"},
                           {{"1c1", AtomicClockConstraintT<std::less<Time>>{1}}}}}};
	TA   ta2{{SingleLocation{"2l0"}, SingleLocation{"2l1"}},
         {"2a", "2b"},
         SingleLocation{"2l0"},
         {SingleLocation{"2l1"}},
         {"2c1"},
         {SingleTransition{SingleLocation{"2l0"},
                           "2a",
                           SingleLocation{"2l1"},
                           {{"2c1", AtomicClockConstraintT<std::less<Time>>{2}}}}}};
	TA   ta3{{SingleLocation{"3l0"}, SingleLocation{"3l1"}},
         {"3a", "3b"},
         SingleLocation{"3l0"},
         {SingleLocation{"3l1"}},
         {"3c1"},
         {SingleTransition{SingleLocation{"3l0"},
                           "3a",
                           SingleLocation{"3l1"},
                           {{"3c1", AtomicClockConstraintT<std::less<Time>>{3}}}}}};
	auto product = get_product<std::string, std::string>({ta1, ta2, ta3});
	CHECK(product.get_locations()
	      == std::set<ProductLocation>{ProductLocation{{"1l0", "2l0", "3l0"}},
	                                   ProductLocation{{"1l0", "2l0", "3l1"}},
	                                   ProductLocation{{"1l0", "2l1", "3l0"}},
	                                   ProductLocation{{"1l0", "2l1", "3l1"}},
	                                   ProductLocation{{"1l1", "2l0", "3l0"}},
	                                   ProductLocation{{"1l1", "2l0", "3l1"}},
	                                   ProductLocation{{"1l1", "2l1", "3l0"}},
	                                   ProductLocation{{"1l1", "2l1", "3l1"}}});
	CHECK(product.get_final_locations()
	      == std::set<ProductLocation>{ProductLocation{{"1l1", "2l1", "3l1"}}});
	CHECK(product.get_clocks() == std::set<std::string>{"1c1", "2c1", "3c1"});
	CHECK(product.get_transitions()
	      == std::multimap<ProductLocation, ProductTransition>{
	        {{ProductLocation{{"1l0", "2l0", "3l0"}},
	          ProductTransition{ProductLocation{{"1l0", "2l0", "3l0"}},
	                            "1a",
	                            ProductLocation{{"1l1", "2l0", "3l0"}},
	                            {{"1c1", AtomicClockConstraintT<std::less<Time>>{1}}}}},
	         {ProductLocation{{"1l0", "2l0", "3l1"}},
	          ProductTransition{ProductLocation{{"1l0", "2l0", "3l1"}},
	                            "1a",
	                            ProductLocation{{"1l1", "2l0", "3l1"}},
	                            {{"1c1", AtomicClockConstraintT<std::less<Time>>{1}}}}},
	         {ProductLocation{{"1l0", "2l1", "3l0"}},
	          ProductTransition{ProductLocation{{"1l0", "2l1", "3l0"}},
	                            "1a",
	                            ProductLocation{{"1l1", "2l1", "3l0"}},
	                            {{"1c1", AtomicClockConstraintT<std::less<Time>>{1}}}}},
	         {ProductLocation{{"1l0", "2l1", "3l1"}},
	          ProductTransition{ProductLocation{{"1l0", "2l1", "3l1"}},
	                            "1a",
	                            ProductLocation{{"1l1", "2l1", "3l1"}},
	                            {{"1c1", AtomicClockConstraintT<std::less<Time>>{1}}}}},
	         {ProductLocation{{"1l0", "2l0", "3l0"}},
	          ProductTransition{ProductLocation{{"1l0", "2l0", "3l0"}},
	                            "2a",
	                            ProductLocation{{"1l0", "2l1", "3l0"}},
	                            {{"2c1", AtomicClockConstraintT<std::less<Time>>{2}}}}},
	         {ProductLocation{{"1l0", "2l0", "3l1"}},
	          ProductTransition{ProductLocation{{"1l0", "2l0", "3l1"}},
	                            "2a",
	                            ProductLocation{{"1l0", "2l1", "3l1"}},
	                            {{"2c1", AtomicClockConstraintT<std::less<Time>>{2}}}}},
	         {ProductLocation{{"1l1", "2l0", "3l0"}},
	          ProductTransition{ProductLocation{{"1l1", "2l0", "3l0"}},
	                            "2a",
	                            ProductLocation{{"1l1", "2l1", "3l0"}},
	                            {{"2c1", AtomicClockConstraintT<std::less<Time>>{2}}}}},
	         {ProductLocation{{"1l1", "2l0", "3l1"}},
	          ProductTransition{ProductLocation{{"1l1", "2l0", "3l1"}},
	                            "2a",
	                            ProductLocation{{"1l1", "2l1", "3l1"}},
	                            {{"2c1", AtomicClockConstraintT<std::less<Time>>{2}}}}},
	         {ProductLocation{{"1l0", "2l0", "3l0"}},
	          ProductTransition{ProductLocation{{"1l0", "2l0", "3l0"}},
	                            "3a",
	                            ProductLocation{{"1l0", "2l0", "3l1"}},
	                            {{"3c1", AtomicClockConstraintT<std::less<Time>>{3}}}}},
	         {ProductLocation{{"1l0", "2l1", "3l0"}},
	          ProductTransition{ProductLocation{{"1l0", "2l1", "3l0"}},
	                            "3a",
	                            ProductLocation{{"1l0", "2l1", "3l1"}},
	                            {{"3c1", AtomicClockConstraintT<std::less<Time>>{3}}}}},
	         {ProductLocation{{"1l1", "2l0", "3l0"}},
	          ProductTransition{ProductLocation{{"1l1", "2l0", "3l0"}},
	                            "3a",
	                            ProductLocation{{"1l1", "2l0", "3l1"}},
	                            {{"3c1", AtomicClockConstraintT<std::less<Time>>{3}}}}},
	         {ProductLocation{{"1l1", "2l1", "3l0"}},
	          ProductTransition{ProductLocation{{"1l1", "2l1", "3l0"}},
	                            "3a",
	                            ProductLocation{{"1l1", "2l1", "3l1"}},
	                            {{"3c1", AtomicClockConstraintT<std::less<Time>>{3}}}}}}});
	CHECK(product.accepts_word({{"1a", 0}, {"2a", 1}, {"3a", 2}}));
	CHECK(product.accepts_word({{"3a", 0}, {"2a", 0}, {"1a", 0}}));
	CHECK(!product.accepts_word({{"1a", 0}, {"2a", 1}}));
	CHECK(!product.accepts_word({{"3a", 0}, {"2a", 1}, {"3a", 2}}));
	CHECK(!product.accepts_word({{"1a", 0}, {"2a", 3}, {"3a", 4}}));
}
} // namespace
