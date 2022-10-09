/***************************************************************************
 *  benchmark_robot.cpp - Control a robot's camera
 *
 *  Created:   Mon 26 Jul 18:48:06 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/
#include "gocos/golog_adapter.h"
#include "gocos/golog_program.h"
#include "golog_robot.h"
#include "heuristics_generator.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "search/create_controller.h"
#include "search/heuristics.h"
#include "search/search.h"
#include "visualization/tree_to_graphviz.h"

#include <benchmark/benchmark.h>

using namespace tacos;

using logic::AtomicProposition;
using search::GologProgram;
using TreeSearch = tacos::search::
  TreeSearch<search::GologLocation, std::string, std::string, true, GologProgram, true>;

enum class Mode {
	SIMPLE,
	WEIGHTED,
};

std::set<std::string>
unwrap(const std::set<AtomicProposition<std::set<std::string>>> &input)
{
	std::set<std::string> res;
	for (const auto &i : input) {
		for (const auto &s : i.ap_) {
			res.insert(s);
		}
	}
	return res;
}

static void
BM_GologRobot(benchmark::State &state, Mode mode)
{
	spdlog::set_level(spdlog::level::err);
	spdlog::set_pattern("%t %v");
	const unsigned int camtime = 2;
	const auto [program_string, spec, controller_actions, environment_actions] =
	  create_robot_problem();
	auto         ata = mtl_ata_translation::translate<std::string, std::set<std::string>, true>(spec);
	const auto   relevant_fluents = unwrap(ata.get_alphabet());
	GologProgram program(program_string, relevant_fluents);
	const auto   K = camtime;

	std::size_t tree_size        = 0;
	std::size_t pruned_tree_size = 0;
	std::size_t controller_size  = 0;

	std::unique_ptr<search::Heuristic<long, TreeSearch::Node>> heuristic;

	for (auto _ : state) {
		switch (mode) {
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

// Range all over all heuristics individually.
BENCHMARK_CAPTURE(BM_GologRobot, single_heuristic, Mode::SIMPLE)
  ->DenseRange(0, 5, 1)
  ->MeasureProcessCPUTime()
  ->Unit(benchmark::kSecond)
  ->UseRealTime();
// Weighted heuristics.
BENCHMARK_CAPTURE(BM_GologRobot, weighted, Mode::WEIGHTED)
  ->ArgsProduct({benchmark::CreateRange(1, 16, 2),
                 benchmark::CreateRange(1, 16, 2),
                 benchmark::CreateDenseRange(0, 2, 1)})
  ->MeasureProcessCPUTime()
  ->Unit(benchmark::kSecond)
  ->UseRealTime();
