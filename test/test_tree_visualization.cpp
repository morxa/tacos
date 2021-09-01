/***************************************************************************
 *  test_tree_visualization.cpp - Test search tree visualization
 *
 *  Created:   Thu 15 Apr 20:54:49 CEST 2021
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
#include "mtl/MTLFormula.h"

#include <search/canonical_word.h>
#include <search/search_tree.h>
#include <visualization/interactive_tree_to_graphviz.h>
#include <visualization/tree_to_graphviz.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <fstream>

namespace {

using namespace tacos;

using ATARegionState  = search::ATARegionState<std::string>;
using CanonicalABWord = search::CanonicalABWord<std::string, std::string>;
using Location        = automata::ta::Location<std::string>;
using Node            = search::SearchTreeNode<std::string, std::string>;
using TARegionState   = search::TARegionState<std::string>;

using search::LabelReason;
using search::NodeLabel;

using Catch::Matchers::Contains;

auto
create_test_graph()
{
	auto create_test_node = [](const std::set<CanonicalABWord> &      words,
	                           const std::map<std::pair<search::RegionIndex, std::string>,
	                                          std::shared_ptr<Node>> &children = {}) {
		auto node          = std::make_shared<Node>(words);
		node->is_expanding = true;
		for (const auto &[action, child] : children) {
			node->add_child(action, child);
			child->parents = {node.get()};
		}
		return node;
	};
	const logic::MTLFormula            a{logic::AtomicProposition<std::string>{"a"}};
	const logic::MTLFormula            b{logic::AtomicProposition<std::string>{"b"}};
	std::vector<std::shared_ptr<Node>> children;
	auto                               n1c1 = create_test_node(
    {{{TARegionState{Location{"l0"}, "x", 0}}, {TARegionState{Location{"l0"}, "y", 2}}}});
	auto n1 = create_test_node({{{TARegionState{Location{"l0"}, "x", 0}},
	                             {TARegionState{Location{"l0"}, "y", 1}}}},
	                           {{{1, "d"}, n1c1}});
	auto n2 = create_test_node(
	  {{{TARegionState{Location{"l0"}, "x", 1}}, {TARegionState{Location{"l0"}, "y", 2}}}});
	auto n3 = create_test_node(
	  {{{TARegionState{Location{"l0"}, "x", 1}}, {TARegionState{Location{"l0"}, "y", 2}}},
	   {{ATARegionState{a.until(b), 1}, TARegionState{Location{"l0"}, "x", 1}},
	    {TARegionState{Location{"l0"}, "y", 2}}}});
	auto root          = create_test_node({{{TARegionState{Location{"l0"}, "x", 0},
                                  TARegionState{Location{"l0"}, "y", 0}}}},
                               {{{1, "a"}, n1}, {{2, "b"}, n2}, {{3, "c"}, n3}});
	root->label        = NodeLabel::TOP;
	root->label_reason = LabelReason::GOOD_CONTROLLER_ACTION_FIRST;
	n1->label          = NodeLabel::TOP;
	n1->label_reason   = LabelReason::DEAD_NODE;
	n2->label          = NodeLabel::BOTTOM;
	n2->label_reason   = LabelReason::NO_BAD_ENV_ACTION;
	n3->label          = NodeLabel::BOTTOM;
	n3->label_reason   = LabelReason::BAD_ENV_ACTION_FIRST;
	return root;
}
std::string
read_file(const std::filesystem::path &file)
{
	std::ifstream      ifs(file);
	std::ostringstream sstr;
	sstr << ifs.rdbuf();
	return sstr.str();
}

TEST_CASE("Search tree visualization", "[search][visualization]")
{
	auto root  = create_test_graph();
	auto graph = tacos::visualization::search_tree_to_graphviz(*root);
	graph.render_to_file("test_tree_visualization.png");
	const auto dot = graph.to_dot();

	// All nodes should have shape 'record'.
	CHECK_THAT(dot, Contains("shape=record"));

	// Check that all nodes have the expected labels.
	CHECK_THAT(dot,
	           Contains(
	             R"dot(label="{good controller action first}|{ { (l0, x, 0), (l0, y, 0) } }")dot"));
	CHECK_THAT(dot, Contains(R"dot(label="{dead node}|{ { (l0, x, 0) }|{ (l0, y, 1) } }")dot"));
	CHECK_THAT(dot,
	           Contains(R"dot(label="{no bad env action}|{ { (l0, x, 1) }|{ (l0, y, 2) } }")dot"));
	CHECK_THAT(
	  dot,
	  Contains(
	    R"dot(label="{bad env action first}|{ { (l0, x, 1) }|{ (l0, y, 2) } }|{ { (l0, x, 1), ((a U b), 1) }|{ (l0, y, 2) } }")dot"));

	// Check that both colors occur, we assume they are the right nodes.
	CHECK_THAT(dot, Contains("color=green"));
	CHECK_THAT(dot, Contains("color=red"));

	// Check that all four edges occur.
	CHECK_THAT(dot,
	           Contains(
	             R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 1) } }")dot"));
	CHECK_THAT(dot,
	           Contains(
	             R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 1) }|{ (l0, y, 2) } }")dot"));
	CHECK_THAT(
	  dot,
	  Contains(
	    R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 1) }|{ (l0, y, 2) } }|{ { (l0, x, 1), ((a U b), 1) }|{ (l0, y, 2) } }")dot"));
	CHECK_THAT(
	  dot,
	  Contains(
	    R"dot("{ { (l0, x, 0) }|{ (l0, y, 1) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 2) } }")dot"));

	// Check that all three actions occur.
	CHECK_THAT(dot, Contains("(1, a)"));
	CHECK_THAT(dot, Contains("(2, b)"));
	CHECK_THAT(dot, Contains("(3, c)"));
	CHECK_THAT(dot, Contains("(1, d)"));
}

TEST_CASE("Interactive visualization", "[visualization]")
{
	auto              root = create_test_graph();
	std::stringstream input;
	char              tmp_filename[] = "search_graph_XXXXXX.dot";
	mkstemps(tmp_filename, 4);
	std::filesystem::path tmp_file(tmp_filename);
	SECTION("Root node")
	{
		input << "q" << std::endl;
		visualization::search_tree_to_graphviz_interactive(root.get(), tmp_file, input);
		std::string dot = read_file(tmp_file);
		CAPTURE(dot);
		CHECK_THAT(dot, Contains("{ { (l0, x, 0), (l0, y, 0) } }"));
	}
	SECTION("First child")
	{
		input << "i" << std::endl << "0" << std::endl << "q" << std::endl;
		visualization::search_tree_to_graphviz_interactive(root.get(), tmp_file, input);
		std::string dot = read_file(tmp_file);
		CAPTURE(dot);
		CHECK_THAT(
		  dot,
		  Contains(R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 1) } }")dot"));
	}
	SECTION("Undo")
	{
		input << "i\n0\nu\nq" << std::endl;
		visualization::search_tree_to_graphviz_interactive(root.get(), tmp_file, input);
		std::string dot = read_file(tmp_file);
		CAPTURE(dot);
		CHECK_THAT(
		  dot,
		  !Contains(
		    R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 1) } }")dot"));
	}
	SECTION("All of root's children")
	{
		input << "i\n*\nq" << std::endl;
		visualization::search_tree_to_graphviz_interactive(root.get(), tmp_file, input);
		std::string dot = read_file(tmp_file);
		CAPTURE(dot);
		CHECK_THAT(
		  dot,
		  Contains(R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 1) } }")dot"));
		CHECK_THAT(
		  dot,
		  Contains(R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 1) }|{ (l0, y, 2) } }")dot"));
		CHECK_THAT(
		  dot,
		  Contains(
		    R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 1) }|{ (l0, y, 2) } }|{ { (l0, x, 1), ((a U b), 1) }|{ (l0, y, 2) } }")dot"));
		CHECK_THAT(
		  dot,
		  !Contains(
		    R"dot("{ { (l0, x, 0) }|{ (l0, y, 1) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 2) } }")dot"));
		CHECK_THAT(dot, !Contains("(1, d)"));
	}
	SECTION("Child of the first child with separate selection and navigation")
	{
		input << "i\n0\nn\n0\ni\n0\nq" << std::endl;
		visualization::search_tree_to_graphviz_interactive(root.get(), tmp_file, input);
		std::string dot = read_file(tmp_file);
		CAPTURE(dot);
		CHECK_THAT(
		  dot,
		  Contains(R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 1) } }")dot"));
		CHECK_THAT(
		  dot,
		  !Contains(
		    R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 1) }|{ (l0, y, 2) } }")dot"));
		CHECK_THAT(
		  dot,
		  !Contains(
		    R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 1) }|{ (l0, y, 2) } }|{ { (l0, x, 1), ((a U b), 1) }|{ (l0, y, 2) } }")dot"));
		CHECK_THAT(
		  dot,
		  Contains(
		    R"dot("{ { (l0, x, 0) }|{ (l0, y, 1) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 2) } }")dot"));
	}
	SECTION("Child of the first child with simultaneous selection and navigation")
	{
		input << "a\n0\n0\nq" << std::endl;
		visualization::search_tree_to_graphviz_interactive(root.get(), tmp_file, input);
		std::string dot = read_file(tmp_file);
		CAPTURE(dot);
		CHECK_THAT(
		  dot,
		  Contains(R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 1) } }")dot"));
		CHECK_THAT(
		  dot,
		  !Contains(
		    R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 1) }|{ (l0, y, 2) } }")dot"));
		CHECK_THAT(
		  dot,
		  !Contains(
		    R"dot("{ { (l0, x, 0), (l0, y, 0) } }" -> "{ { (l0, x, 1) }|{ (l0, y, 2) } }|{ { (l0, x, 1), ((a U b), 1) }|{ (l0, y, 2) } }")dot"));
		CHECK_THAT(
		  dot,
		  Contains(
		    R"dot("{ { (l0, x, 0) }|{ (l0, y, 1) } }" -> "{ { (l0, x, 0) }|{ (l0, y, 2) } }")dot"));
	}
	remove(tmp_file);
}

} // namespace
