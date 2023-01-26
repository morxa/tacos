/***************************************************************************
 *  benchmark_golog_household.cpp - Benchmark Golog household scenario
 *
 *  Created:   Wed 19 Oct 23:37:21 CEST 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/
#include "benchmark_golog.h"
#include "golog_adapter/golog_adapter.h"
#include "golog_adapter/golog_program.h"
#include "golog_household.h"
#include "heuristics_generator.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "search/create_controller.h"
#include "search/heuristics.h"
#include "search/search.h"
#include "visualization/tree_to_graphviz.h"

#include <benchmark/benchmark.h>

using namespace tacos;

using search::GologProgram;
using TreeSearch = tacos::search::
  TreeSearch<search::GologLocation, std::string, std::string, true, GologProgram, true>;

static void
BM_GologHousehold(benchmark::State &state)
{
	spdlog::set_level(spdlog::level::err);
	spdlog::set_pattern("%t %v");
	unsigned int align_time;
	align_time = state.range(0);
	const auto [program_string, spec, controller_actions, environment_actions] =
	  create_household_problem(align_time);
	auto         ata = mtl_ata_translation::translate<std::string, std::set<std::string>, true>(spec);
	const auto   relevant_fluents = unwrap(ata.get_alphabet());
	GologProgram program(program_string, relevant_fluents, false, {"end(align(table))"});
	const auto   K = align_time;

	std::size_t tree_size        = 0;
	std::size_t pruned_tree_size = 0;
	std::size_t controller_size  = 0;

	std::unique_ptr<search::Heuristic<long, TreeSearch::Node>> heuristic =
	  std::make_unique<search::DfsHeuristic<long, TreeSearch::Node>>();

	for (auto _ : state) {
		TreeSearch search{
		  &program, &ata, controller_actions, environment_actions, K, true, true, std::move(heuristic)};

		search.build_tree(false);
		search.label();
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
	state.counters["tree_size"] =
	  benchmark::Counter(static_cast<double>(tree_size), benchmark::Counter::kAvgIterations);
	state.counters["pruned_tree_size"] =
	  benchmark::Counter(static_cast<double>(pruned_tree_size), benchmark::Counter::kAvgIterations);
	state.counters["controller_size"] =
	  benchmark::Counter(static_cast<double>(controller_size), benchmark::Counter::kAvgIterations);
}

BENCHMARK(BM_GologHousehold)
  ->DenseRange(1, 4, 1)
  ->MeasureProcessCPUTime()
  ->Unit(benchmark::kSecond)
  ->UseRealTime();
