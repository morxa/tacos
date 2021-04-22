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

#include "synchronous_product/heuristics.h"

#include <catch2/catch_test_macros.hpp>

namespace {

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

} // namespace
