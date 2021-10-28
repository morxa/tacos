/***************************************************************************
 *  benchmark_railroad.cpp - Benchmarking the railroad example
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

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_product.h"
#include "automata/ta_regions.h"
#include "heuristics_generator.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "railroad.h"
#include "search/canonical_word.h"
#include "search/create_controller.h"
#include "search/heuristics.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "search/synchronous_product.h"

#include <benchmark/benchmark.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <stdexcept>

enum class Mode {
	SIMPLE,
	WEIGHTED,
	SCALED,
};

using namespace tacos;

using Location   = automata::ta::Location<std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;
using automata::Time;
using F          = logic::MTLFormula<std::string>;
using AP         = logic::AtomicProposition<std::string>;
using TreeSearch = search::TreeSearch<std::vector<std::string>, std::string>;

static void
BM_Railroad(benchmark::State &state, Mode mode, bool multi_threaded = true)
{
	spdlog::set_level(spdlog::level::err);
	spdlog::set_pattern("%t %v");
	std::vector<Time> distances;
	switch (mode) {
	case Mode::SIMPLE:
	case Mode::WEIGHTED: distances = {2, 2}; break;
	case Mode::SCALED:
		for (std::size_t i = 0; i < 3; ++i) {
			if (state.range(i) > 0) {
				distances.push_back(state.range(i));
			}
		}
		break;
	}
	const auto   problem             = create_crossing_problem(distances);
	auto         plant               = std::get<0>(problem);
	auto         spec                = std::get<1>(problem);
	auto         controller_actions  = std::get<2>(problem);
	auto         environment_actions = std::get<3>(problem);
	std::set<AP> actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               inserter(actions, end(actions)));
	auto               ata = mtl_ata_translation::translate(spec, actions);
	const unsigned int K   = std::max(plant.get_largest_constant(), spec.get_largest_constant());

	std::size_t tree_size        = 0;
	std::size_t pruned_tree_size = 0;
	std::size_t controller_size  = 0;
	std::size_t plant_size       = 0;

	std::unique_ptr<search::Heuristic<long, TreeSearch::Node>> heuristic;

	for (auto _ : state) {
		switch (mode) {
		case Mode::SCALED:
			heuristic = generate_heuristic<TreeSearch::Node>(16, 4, environment_actions, 1);
			break;
		case Mode::WEIGHTED:
			heuristic = generate_heuristic<TreeSearch::Node>(state.range(0),
			                                                 state.range(1),
			                                                 environment_actions,
			                                                 state.range(2));
			break;
		case (Mode::SIMPLE):
			if (state.range(0) == 0) {
				heuristic = std::make_unique<search::BfsHeuristic<long, TreeSearch::Node>>();
			} else if (state.range(0) == 1) {
				heuristic = std::make_unique<search::DfsHeuristic<long, TreeSearch::Node>>();
			} else if (state.range(0) == 2) {
				heuristic = std::make_unique<search::NumCanonicalWordsHeuristic<long, TreeSearch::Node>>();
			} else if (state.range(0) == 3) {
				heuristic = std::make_unique<
				  search::PreferEnvironmentActionHeuristic<long, TreeSearch::Node, std::string>>(
				  environment_actions);
			} else if (state.range(0) == 4) {
				heuristic = std::make_unique<search::TimeHeuristic<long, TreeSearch::Node>>();
			} else if (state.range(0) == 5) {
				heuristic = std::make_unique<search::RandomHeuristic<long, TreeSearch::Node>>(time(NULL));
			} else {
				throw std::invalid_argument("Unexpected argument");
			}
			break;
		}
		TreeSearch search{
		  &plant, &ata, controller_actions, environment_actions, K, true, true, std::move(heuristic)};

		search.build_tree(multi_threaded);
		search.label();
		plant_size += plant.get_locations().size();
		tree_size += search.get_size();
		std::for_each(std::begin(search.get_nodes()),
		              std::end(search.get_nodes()),
		              [&pruned_tree_size](const auto &node) {
			              if (node.second->label != search::NodeLabel::CANCELED
			                  && node.second->label != search::NodeLabel::UNLABELED) {
				              pruned_tree_size += 1;
			              }
		              });
		auto controller = controller_synthesis::create_controller(
		  search.get_root(), controller_actions, environment_actions, K, true);
		controller_size += controller.get_locations().size();
	}
	state.counters["tree_size"] = benchmark::Counter(tree_size, benchmark::Counter::kAvgIterations);
	state.counters["pruned_tree_size"] =
	  benchmark::Counter(pruned_tree_size, benchmark::Counter::kAvgIterations);
	state.counters["controller_size"] =
	  benchmark::Counter(controller_size, benchmark::Counter::kAvgIterations);
	state.counters["plant_size"] = benchmark::Counter(plant_size, benchmark::Counter::kAvgIterations);
}

// Range all over all heuristics individually.
BENCHMARK_CAPTURE(BM_Railroad, single_heuristic, Mode::SIMPLE)
  ->DenseRange(0, 5, 1)
  ->MeasureProcessCPUTime()
  ->UseRealTime();
// Single-threaded.
BENCHMARK_CAPTURE(BM_Railroad, single_heuristic_single_thread, Mode::SIMPLE, false)
  ->DenseRange(0, 5, 1)
  ->MeasureProcessCPUTime()
  ->UseRealTime();
// Single-threaded with weighted heuristics.
BENCHMARK_CAPTURE(BM_Railroad, weighted_single_thread, Mode::WEIGHTED, false)
  ->Args({16, 4, 1})
  ->MeasureProcessCPUTime()
  ->UseRealTime();
// Weighted heuristics.
BENCHMARK_CAPTURE(BM_Railroad, weighted, Mode::WEIGHTED)
  ->ArgsProduct({benchmark::CreateRange(1, 16, 2),
                 benchmark::CreateRange(1, 16, 2),
                 benchmark::CreateDenseRange(0, 2, 1)})
  ->MeasureProcessCPUTime()
  ->UseRealTime();
// Different distances
BENCHMARK_CAPTURE(BM_Railroad, scaled, Mode::SCALED)
  ->ArgsProduct({benchmark::CreateRange(1, 8, 2), benchmark::CreateRange(1, 8, 2), {0}})
  ->MeasureProcessCPUTime()
  ->UseRealTime();
BENCHMARK_CAPTURE(BM_Railroad, scaled, Mode::SCALED)
  ->Args({1, 1, 1})
  ->Args({2, 1, 1})
  ->Args({2, 2, 2})
  ->MeasureProcessCPUTime()
  ->UseRealTime();
