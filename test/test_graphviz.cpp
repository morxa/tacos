/***************************************************************************
 *  test_graphviz.cpp - Test graphviz wrapper
 *
 *  Created:   Thu 15 Apr 10:20:22 CEST 2021
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

#include "catch2/matchers/catch_matchers_string.hpp"

#include <utilities/graphviz/graphviz.h>

#include <catch2/catch_test_macros.hpp>

namespace {

using Catch::Matchers::Contains;
using utilities::graphviz::Graph;

TEST_CASE("Create a graphviz graph", "[utilities][graphviz]")
{
	Graph g{};
	auto  n1 = g.add_node("node 1");
	auto  n2 = g.add_node("node 2");
	auto  n3 = g.add_node("node 3");
	auto  n4 = g.add_node("node 4");
	n2.set_property("color", "red");
	n3.set_property("color", "green");
	g.add_edge(n1, n2);
	g.add_edge(n2, n3, "foo bar");
	g.add_edge(n2, n4, "foo baz");
	const auto dot = g.to_dot();
	CHECK_THAT(dot, Contains("shape=record"));
	CHECK_THAT(dot, Contains("\"node 1\""));
	CHECK_THAT(dot, Contains("\"node 2\""));
	CHECK_THAT(dot, Contains("\"node 3\""));
	CHECK_THAT(dot, Contains("\"node 4\""));
	CHECK_THAT(dot, Contains("1 -> 2"));
	CHECK_THAT(dot, Contains("2 -> 3"));
	CHECK_THAT(dot, Contains("2 -> 4"));
	CHECK_THAT(dot, Contains("label=\"foo bar\""));
	CHECK_THAT(dot, Contains("label=\"foo baz\""));
	CHECK_THAT(dot, Contains("color=red"));
	CHECK_THAT(dot, Contains("color=green"));
	g.render_to_file("graphviz.png");
	CHECK_THROWS(g.render_to_file("nosuffix"));
}

} // namespace
