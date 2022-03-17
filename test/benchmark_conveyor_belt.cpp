/***************************************************************************
 *  benchmark_converyor_belt - case study with a simple conveyor belt model
 *
 *  Created: Mon 26 Jul 2021 20:06:42 CEST 13:00
 *  Copyright  2021  Stefan Schupp <stefan.schupp@tuwien.ac.at>
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
 *  Read the full text in the LICENSE.GPL file in the doc directory.
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
#include "search/ta_adapter.h"

#include <benchmark/benchmark.h>

#include <string_view>

using namespace tacos;

using Location   = automata::ta::Location<std::string_view>;
using TA         = automata::ta::TimedAutomaton<std::string_view, std::string_view>;
using Transition = automata::ta::Transition<std::string_view, std::string_view>;
using F          = logic::MTLFormula<std::string_view>;
using AP         = logic::AtomicProposition<std::string_view>;
using TreeSearch = search::TreeSearch<automata::ta::Location<std::string_view>, std::string_view>;

static void
BM_ConveyorBelt(benchmark::State &state, bool weighted = true, bool multi_threaded = true)
{
	Location l_no{"NO"};
	Location l_st{"ST"};
	Location l_sp{"SP"};

	std::set<std::string_view> environment_actions{"release", "resume", "stuck"};
	std::set<std::string_view> controller_actions{"move", "stop"};
	std::set<std::string_view> actions;
	std::set_union(std::begin(environment_actions),
	               std::end(environment_actions),
	               std::begin(controller_actions),
	               std::end(controller_actions),
	               std::inserter(actions, std::begin(actions)));

	// the conveyor belt plant
	TA plant{{l_no, l_st, l_sp},
	         actions,
	         l_no,
	         {l_no},
	         {"move_timer", "stuck_timer"},
	         {Transition{l_no,
	                     "move",
	                     l_no,
	                     {{"move_timer",
	                       automata::AtomicClockConstraintT<std::greater_equal<Time>>{1}}},
	                     {"move_timer"}},
	          Transition{l_no, "stuck", l_st, {}, {"stuck_timer"}},
	          Transition{l_no, "stop", l_sp},
	          Transition{l_st, "release", l_no},
	          Transition{l_sp, "resume", l_no}}};

	// the specification
	const auto move_f    = F{AP{"move"}};
	const auto release_f = F{AP{"release"}};
	const auto stuck_f   = F{AP{"stuck"}};
	const auto stop_f    = F{AP{"stop"}};
	const auto resume_f  = F{AP{"resume"}};
	const auto spec      = finally(release_f && finally(move_f, logic::TimeInterval(0, 2)))
	                  || finally(stop_f && (!stuck_f).until(stop_f)) || (!stuck_f).until(stop_f);
	// || finally(globally(!move_f)); // cannot be satisfied as we cannot enforce 'release'

	auto ata = mtl_ata_translation::translate(
	  spec, {AP{"move"}, AP{"release"}, AP{"stuck"}, AP{"stop"}, AP{"resume"}});
	const unsigned int K = std::max(plant.get_largest_constant(), spec.get_largest_constant());

	std::size_t tree_size        = 0;
	std::size_t pruned_tree_size = 0;
	std::size_t controller_size  = 0;
	std::size_t plant_size       = 0;

	for (auto _ : state) {
		std::unique_ptr<search::Heuristic<long, TreeSearch::Node>> heuristic;
		if (weighted) {
			heuristic = generate_heuristic<TreeSearch::Node, std::string_view>(state.range(0),
			                                                                   state.range(1),
			                                                                   environment_actions,
			                                                                   state.range(2));
		} else {
			if (state.range(0) == 0) {
				heuristic = std::make_unique<search::BfsHeuristic<long, TreeSearch::Node>>();
			} else if (state.range(0) == 1) {
				heuristic = std::make_unique<search::DfsHeuristic<long, TreeSearch::Node>>();
			} else if (state.range(0) == 2) {
				heuristic = std::make_unique<search::NumCanonicalWordsHeuristic<long, TreeSearch::Node>>();
			} else if (state.range(0) == 3) {
				heuristic = std::make_unique<
				  search::PreferEnvironmentActionHeuristic<long, TreeSearch::Node, std::string_view>>(
				  environment_actions);
			} else if (state.range(0) == 4) {
				heuristic = std::make_unique<search::TimeHeuristic<long, TreeSearch::Node>>();
			} else if (state.range(0) == 5) {
				heuristic = std::make_unique<search::RandomHeuristic<long, TreeSearch::Node>>(time(NULL));
			} else {
				throw std::invalid_argument("Unexpected argument");
			}
		}
		TreeSearch search{&plant,
		                  &ata,
		                  controller_actions,
		                  environment_actions,
		                  K,
		                  true,
		                  true,
		                  generate_heuristic<TreeSearch::Node, std::string_view>()};
		search.build_tree(multi_threaded);
		search.label();
		auto controller = controller_synthesis::create_controller(search.get_root(),
		                                                          controller_actions,
		                                                          environment_actions,
		                                                          K);
		tree_size += search.get_size();
		std::for_each(std::begin(search.get_nodes()),
		              std::end(search.get_nodes()),
		              [&pruned_tree_size](const auto &node) {
			              if (node.second->label != search::NodeLabel::CANCELED
			                  && node.second->label != search::NodeLabel::UNLABELED) {
				              pruned_tree_size += 1;
			              }
		              });
		plant_size += plant.get_locations().size();
		controller_size += controller.get_locations().size();
	}

	state.counters["tree_size"] = benchmark::Counter(tree_size, benchmark::Counter::kAvgIterations);
	state.counters["pruned_tree_size"] =
	  benchmark::Counter(pruned_tree_size, benchmark::Counter::kAvgIterations);
	state.counters["controller_size"] =
	  benchmark::Counter(controller_size, benchmark::Counter::kAvgIterations);
	state.counters["plant_size"] = benchmark::Counter(plant_size, benchmark::Counter::kAvgIterations);
}

BENCHMARK_CAPTURE(BM_ConveyorBelt, single_heuristic, false)
  ->DenseRange(0, 5, 1)
  ->MeasureProcessCPUTime()
  ->UseRealTime();
BENCHMARK_CAPTURE(BM_ConveyorBelt, single_heuristic_single_thread, false, false)
  ->DenseRange(0, 5, 1)
  ->MeasureProcessCPUTime()
  ->UseRealTime();
// Single-threaded with weighted heuristics.
BENCHMARK_CAPTURE(BM_ConveyorBelt, weighted_single_thread, true, false)
  ->Args({16, 4, 1})
  ->MeasureProcessCPUTime()
  ->UseRealTime();
BENCHMARK_CAPTURE(BM_ConveyorBelt, weighted, true)
  ->ArgsProduct({benchmark::CreateRange(1, 16, 2),
                 benchmark::CreateRange(1, 16, 2),
                 benchmark::CreateDenseRange(0, 2, 1)})
  ->MeasureProcessCPUTime()
  ->UseRealTime();
