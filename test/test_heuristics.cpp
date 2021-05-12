/***************************************************************************
 *  test_heuristics.cpp - Test search tree heuristic functions
 *
 *  Created:   Tue 23 Mar 09:08:56 CET 2021
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

#include "automata/ta_regions.h"
#include "synchronous_product/canonical_word.h"
#include "synchronous_product/heuristics.h"
#include "synchronous_product/search_tree.h"

#include <catch2/catch_test_macros.hpp>

namespace {

using Node = synchronous_product::SearchTreeNode<std::string, std::string>;

TEST_CASE("Test BFS heuristic", "[search][heuristics]")
{
	synchronous_product::BfsHeuristic<long, std::string, std::string> bfs{};
	// The heuristic does not care about the actual node, we can just give it nullptrs.
	long h1 = bfs.compute_cost(nullptr);
	long h2 = bfs.compute_cost(nullptr);
	long h3 = bfs.compute_cost(nullptr);
	CHECK(h1 < h2);
	CHECK(h2 < h3);
}
TEST_CASE("Test DFS heuristic", "[search][heuristics]")
{
	synchronous_product::DfsHeuristic<long, std::string, std::string> dfs{};
	// The heuristic does not care about the actual node, we can just give it nullptrs.
	long h1 = dfs.compute_cost(nullptr);
	long h2 = dfs.compute_cost(nullptr);
	long h3 = dfs.compute_cost(nullptr);
	CHECK(h1 > h2);
	CHECK(h2 > h3);
}

TEST_CASE("Test time heuristic", "[search][heuristics]")
{
	spdlog::set_level(spdlog::level::debug);
	synchronous_product::TimeHeuristic<long, std::string, std::string> h;

	Node root{{}, nullptr, {}};
	CHECK(h.compute_cost(&root) == 0);
	Node c1{{}, &root, {{1, "a1"}}};
	CHECK(h.compute_cost(&c1) == 1);
	Node c2{{}, &root, {{3, "a1"}, {4, "b"}}};
	CHECK(h.compute_cost(&c2) == 3);
	Node cc1{{}, &c1, {{2, "a"}, {4, "a"}}};
	CHECK(h.compute_cost(&cc1) == 3);
	Node cc2{{}, &c2, {{2, "a"}, {4, "a"}}};
	CHECK(h.compute_cost(&cc2) == 5);
}

} // namespace
