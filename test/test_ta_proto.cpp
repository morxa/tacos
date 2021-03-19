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

#include "automata/ta.h"
#include "automata/ta.pb.h"
#include "automata/ta_proto.h"

#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/text_format.h>

#include <catch2/catch_test_macros.hpp>

namespace {

using TimedAutomaton = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition     = automata::ta::Transition<std::string, std::string>;

TEST_CASE("Parse a TA from a proto", "[proto][ta]")
{
	automata::ta::proto::TimedAutomaton proto_ta;
	REQUIRE(google::protobuf::TextFormat::ParseFromString(
	  R"pb(
      locations: "s0"
      locations: "s1"
      final_locations: "s1"
      initial_location: "s0"
      alphabet: "a"
      transitions { source: "s0" target: "s0" symbol: "a" }
    )pb",
	  &proto_ta));
	INFO(proto_ta.DebugString());
	auto ta = automata::ta::parse_proto(proto_ta);

	CHECK(ta.get_locations() == std::set<std::string>{"s0", "s1"});
	CHECK(ta.get_final_locations() == std::set<std::string>{"s1"});
	CHECK(ta.get_alphabet() == std::set<std::string>{"a"});
	CHECK(ta.get_transitions()
	      == std::multimap<std::string, Transition>{{{"s0"}, Transition{"s0", "a", "s0"}}});
}

} // namespace
