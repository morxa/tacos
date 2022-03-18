/***************************************************************************
 *  test_railroad.cpp - A railroad example with location-based constraints
 *
 *  Created:   Mon  1 Mar 17:18:27 CET 2021
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

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_product.h"
#include "automata/ta_regions.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "railroad_location.h"
#include "search/canonical_word.h"
#include "search/create_controller.h"
#include "search/heuristics.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "search/synchronous_product.h"
#include "visualization/ta_to_graphviz.h"
#include "visualization/tree_to_graphviz.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <iterator>

#undef TRUE

namespace {

using namespace tacos;

using automata::Endpoint;
using Location   = automata::ta::Location<std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;
using automata::Time;
using F  = logic::MTLFormula<std::vector<std::string>>;
using AP = logic::AtomicProposition<std::vector<std::string>>;
using search::NodeLabel;
using TreeSearch =
  search::TreeSearch<std::vector<std::string>, std::string, std::vector<std::string>, true>;

std::unique_ptr<search::Heuristic<long, TreeSearch::Node>>
generate_heuristic(long                  weight_canonical_words     = 0,
                   long                  weight_environment_actions = 0,
                   std::set<std::string> environment_actions        = {},
                   long                  weight_time_heuristic      = 1)
{
	using H = search::Heuristic<
	  long,
	  search::SearchTreeNode<std::vector<std::string>, std::string, std::vector<std::string>>>;
	std::vector<std::pair<long, std::unique_ptr<H>>> heuristics;
	heuristics.emplace_back(
	  weight_canonical_words,
	  std::make_unique<search::NumCanonicalWordsHeuristic<long, TreeSearch::Node>>());
	heuristics.emplace_back(
	  weight_environment_actions,
	  std::make_unique<search::PreferEnvironmentActionHeuristic<long, TreeSearch::Node, std::string>>(
	    environment_actions));
	heuristics.emplace_back(weight_time_heuristic,
	                        std::make_unique<search::TimeHeuristic<long, TreeSearch::Node>>());
	return std::make_unique<search::CompositeHeuristic<long, TreeSearch::Node>>(
	  std::move(heuristics));
}

TEST_CASE("Railroad with two crossings", "[.railroad]")
{
	spdlog::set_level(spdlog::level::debug);
	std::vector<Endpoint> distances     = {4};
	const auto            num_crossings = distances.size();
	const auto [plant, spec, controller_actions, environment_actions] =
	  create_crossing_problem(distances);
	std::set<std::string> actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               inserter(actions, end(actions)));
	CAPTURE(spec);
	std::set<logic::AtomicProposition<std::vector<std::string>>> locations;
	std::transform(begin(plant.get_locations()),
	               end(plant.get_locations()),
	               std::inserter(locations, end(locations)),
	               [](const auto location) { return logic::AtomicProposition{location.get()}; });

	auto ata = mtl_ata_translation::translate(spec, locations);
	CAPTURE(plant);
	CAPTURE(ata);
	const unsigned int K = std::max(plant.get_largest_constant(), spec.get_largest_constant());
	TreeSearch         search{
    &plant, &ata, controller_actions, environment_actions, K, true, true, generate_heuristic()};
	SPDLOG_DEBUG("Starting search...");
	search.build_tree(true);
	SPDLOG_DEBUG("Search completed!");
	CHECK(search.get_root()->label == NodeLabel::TOP);
	tacos::visualization::ta_to_graphviz(controller_synthesis::create_controller(search.get_root(),
	                                                                             controller_actions,
	                                                                             environment_actions,
	                                                                             2),
	                                     false)
	  .render_to_file(fmt::format("railroad{}_controller.pdf", num_crossings));
	tacos::visualization::search_tree_to_graphviz(*search.get_root(), true)
	  .render_to_file(fmt::format("railroad{}.svg", num_crossings));
}

} // namespace
