/***************************************************************************
 *  test_ta_visualization.cpp - Test the graphviz visualization of a TA
 *
 *  Created:   Sat 17 Apr 14:45:55 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include "automata/automata.h"
#include "automata/ta.h"
#include "visualization/ta_to_graphviz.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace {

using namespace tacos;

using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using Location   = automata::ta::Location<std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;
using tacos::visualization::ta_to_graphviz;

using Catch::Matchers::ContainsSubstring;
TEST_CASE("Visualize a TA", "[visualization]")
{
	TA ta{{Location{"l0"}, Location{"l1"}},
	      {"a", "b"},
	      Location{"l0"},
	      {Location{"l1"}},
	      {"c", "x"},
	      {Transition{Location{"l0"},
	                  "a",
	                  Location{"l1"},
	                  {{"c", automata::AtomicClockConstraintT<std::less<automata::Time>>{2}}},
	                  {"c"}}}};
	SECTION("Detailed graph")
	{
		auto g   = ta_to_graphviz(ta);
		auto dot = g.to_dot();
		CHECK_THAT(dot, ContainsSubstring("label=l0"));
		CHECK_THAT(dot, ContainsSubstring("label=l1"));
		CHECK_THAT(dot, ContainsSubstring("c < 2"));
		CHECK_THAT(dot, ContainsSubstring("{c}"));
	}
	SECTION("Compact graph")
	{
		auto g   = ta_to_graphviz(ta, false);
		auto dot = g.to_dot();
		CHECK_THAT(dot, ContainsSubstring("label=l0"));
		CHECK_THAT(dot, ContainsSubstring("label=l1"));
		CHECK_THAT(dot, ContainsSubstring("c < 2"));
		CHECK_THAT(dot, ContainsSubstring("{c}"));
		CHECK_THAT(dot, ContainsSubstring("shape=point"));
	}
}

} // namespace
