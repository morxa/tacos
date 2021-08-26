/***************************************************************************
 *  test_ta_proto.cpp - Test the Timed Automaton proto parser
 *
 *  Created:   Fri 19 Mar 13:52:14 CET 2021
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
#include "automata/ta.pb.h"
#include "automata/ta_product.h"
#include "automata/ta_proto.h"

#include <google/protobuf/text_format.h>

#include <catch2/catch_test_macros.hpp>

namespace {

using namespace tacos;

using TimedAutomaton = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition     = automata::ta::Transition<std::string, std::string>;
using Location       = automata::ta::Location<std::string>;

TEST_CASE("TA Proto", "[proto][ta]")
{
	automata::ta::proto::TimedAutomaton proto_ta;
	REQUIRE(google::protobuf::TextFormat::ParseFromString(
	  R"pb(
      locations: "s0"
      locations: "s1"
      locations: "s2"
      final_locations: "s2"
      initial_location: "s0"
      alphabet: "a"
      alphabet: "b"
      clocks: "c1"
      clocks: "c2"
      clocks: "c3"
      clocks: "c4"
      clocks: "c5"
      clocks: "c6"
      transitions {
        source: "s0"
        target: "s1"
        symbol: "a"
        clock_constraints { clock: "c1" operand: LESS comparand: 1 }
        clock_constraints { clock: "c2" operand: LESS_EQUAL comparand: 2 }
        clock_constraints { clock: "c3" operand: EQUAL_TO comparand: 3 }
        clock_resets: "c4"
        clock_resets: "c5"
      }
      transitions {
        source: "s1"
        target: "s2"
        symbol: "b"
        clock_constraints { clock: "c5" operand: GREATER_EQUAL comparand: 5 }
        clock_constraints { clock: "c6" operand: GREATER comparand: 6 }
        clock_resets: "c6"
      }
    )pb",
	  &proto_ta));
	SECTION("Parse a TA from a proto")
	{
		const auto ta = automata::ta::parse_proto(proto_ta);
		CHECK(ta.get_locations() == std::set{Location{"s0"}, Location{"s1"}, Location{"s2"}});
		CHECK(ta.get_initial_location() == Location{"s0"});
		CHECK(ta.get_final_locations() == std::set{Location{"s2"}});
		CHECK(ta.get_alphabet() == std::set<std::string>{"a", "b"});
		CHECK(ta.get_clocks() == std::set<std::string>{"c1", "c2", "c3", "c4", "c5", "c6"});
		CHECK(
		  ta.get_transitions()
		  == std::multimap<Location, Transition>{
		    {{Location{"s0"}},
		     Transition{Location{"s0"},
		                "a",
		                Location{"s1"},
		                {{"c1", automata::AtomicClockConstraintT<std::less<automata::Time>>{1}},
		                 {"c2", automata::AtomicClockConstraintT<std::less_equal<automata::Time>>{2}},
		                 {"c3", automata::AtomicClockConstraintT<std::equal_to<automata::Time>>{3}}},
		                {"c4", "c5"}}},
		    {{Location{"s1"}},
		     Transition{Location{"s1"},
		                "b",
		                Location{"s2"},
		                {{"c5",
		                  automata::AtomicClockConstraintT<std::greater_equal<automata::Time>>{5}},
		                 {"c6", automata::AtomicClockConstraintT<std::greater<automata::Time>>{6}}},
		                {"c6"}}}});
	}

	SECTION("Convert a TA into a proto")
	{
		TimedAutomaton ta{
		  {Location{"s0"}, Location{"s1"}, Location{"s2"}},
		  {"a", "b"},
		  Location{"s0"},
		  {Location{"s2"}},
		  {"c1", "c2", "c3", "c4", "c5", "c6"},
		  {Transition{Location{"s0"},
		              "a",
		              Location{"s1"},
		              {{"c1", automata::AtomicClockConstraintT<std::less<automata::Time>>{1}},
		               {"c2", automata::AtomicClockConstraintT<std::less_equal<automata::Time>>{2}},
		               {"c3", automata::AtomicClockConstraintT<std::equal_to<automata::Time>>{3}}},
		              {"c4", "c5"}},
		   Transition{Location{"s1"},
		              "b",
		              Location{"s2"},
		              {{"c5", automata::AtomicClockConstraintT<std::greater_equal<automata::Time>>{5}},
		               {"c6", automata::AtomicClockConstraintT<std::greater<automata::Time>>{6}}},
		              {"c6"}}}};
		auto generated_proto = automata::ta::ta_to_proto(ta);
		INFO("Proto:\n" << generated_proto.DebugString());
		CHECK(generated_proto.SerializeAsString() == proto_ta.SerializeAsString());
	}
}

TEST_CASE("Parse a TA product from a proto", "[proto][ta]")
{
	automata::ta::proto::ProductAutomaton proto_product;
	REQUIRE(google::protobuf::TextFormat::ParseFromString(
	  R"pb(
      automata {
        locations: "s0"
        locations: "s1"
        initial_location: "s0"
        final_locations: "s1"
        alphabet: "a"
        clocks: "c1"
        transitions {
          source: "s0"
          target: "s1"
          symbol: "a"
          clock_constraints { clock: "c1" operand: LESS comparand: 2 }
        }
      }
      automata {
        locations: "s0"
        locations: "s1"
        initial_location: "s0"
        final_locations: "s1"
        alphabet: "b"
        clocks: "c2"
        transitions {
          source: "s0"
          target: "s1"
          symbol: "b"
          clock_constraints { clock: "c2" operand: GREATER comparand: 2 }
        }
      }
    )pb",
	  &proto_product));
	using ProductLocation = automata::ta::Location<std::vector<std::string>>;
	auto product          = automata::ta::parse_product_proto(proto_product);
	CHECK(product.get_locations()
	      == std::set{
	        ProductLocation{{"s0", "s0"}},
	        ProductLocation{{"s0", "s1"}},
	        ProductLocation{{"s1", "s0"}},
	        ProductLocation{{"s1", "s1"}},
	      });
	CHECK(product.get_initial_location() == ProductLocation{{"s0", "s0"}});
	CHECK(product.get_final_locations() == std::set{ProductLocation{{"s1", "s1"}}});
	CHECK(product.get_clocks() == std::set<std::string>{"c1", "c2"});
}

} // namespace
