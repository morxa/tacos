/***************************************************************************
 *  benchmark_robot.cpp - Control a robot's camera
 *
 *  Created:   Mon 26 Jul 18:48:06 CEST 2021
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
#include "automata/ta_product.h"
#include "heuristics_generator.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "search/create_controller.h"
#include "search/heuristics.h"
#include "search/search.h"

#include <benchmark/benchmark.h>

using namespace tacos;

using Search     = search::TreeSearch<std::vector<std::string>, std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using MTLFormula = logic::MTLFormula<std::string>;
using AP         = logic::AtomicProposition<std::string>;
using automata::AtomicClockConstraintT;

static void
BM_Robot(benchmark::State &state, bool weighted = true, bool multi_threaded = true)
{
	spdlog::set_level(spdlog::level::err);
	spdlog::set_pattern("%t %v");
	const std::set<std::string> robot_actions = {"move", "arrive", "pick", "put"};
	TA                          robot(
    {
      TA::Location{"AT-OUTPUT"},
      TA::Location{"PICKED"},
      TA::Location{"AT-DELIVERY"},
      TA::Location{"PUT"},
      TA::Location{"MOVING-TO-OUTPUT"},
      TA::Location{"MOVING-TO-DELIVERY"},
    },
    robot_actions,
    TA::Location{"MOVING-TO-OUTPUT"},
    {TA::Location{"AT-OUTPUT"}},
    {"c-travel", "cp"},
    {
      TA::Transition(TA::Location{"PICKED"}, "move", TA::Location{"MOVING-TO-DELIVERY"}),
      TA::Transition(TA::Location{"PUT"}, "move", TA::Location{"MOVING-TO-OUTPUT"}),
      TA::Transition(TA::Location{"MOVING-TO-DELIVERY"},
                     "arrive",
                     TA::Location{"AT-DELIVERY"},
                     {{"c-travel", AtomicClockConstraintT<std::equal_to<automata::Time>>{3}}},
                     {"c-travel", "cp"}),
      TA::Transition(TA::Location{"MOVING-TO-OUTPUT"},
                     "arrive",
                     TA::Location{"AT-OUTPUT"},
                     {{"c-travel", AtomicClockConstraintT<std::equal_to<automata::Time>>{3}}},
                     {"c-travel", "cp"}),
      TA::Transition(TA::Location{"AT-OUTPUT"},
                     "pick",
                     TA::Location{"PICKED"},
                     {{"cp", AtomicClockConstraintT<std::equal_to<automata::Time>>{1}}}),
      TA::Transition(TA::Location{"AT-DELIVERY"},
                     "put",
                     TA::Location{"PUT"},
                     {{"cp", AtomicClockConstraintT<std::equal_to<automata::Time>>{1}}}),
    });

	const std::set<std::string> camera_actions = {"switch-on", "switch-off"};
	TA                          camera(
    {TA::Location{"CAMERA-OFF"}, TA::Location{"CAMERA-ON"}},
    camera_actions,
    TA::Location{"CAMERA-OFF"},
    {TA::Location{"CAMERA-OFF"}},
    {"c-camera"},
    {TA::Transition(TA::Location{"CAMERA-OFF"},
                    "switch-on",
                    TA::Location{"CAMERA-ON"},
                    {{"c-camera", AtomicClockConstraintT<std::greater_equal<automata::Time>>{1}}},
                    {"c-camera"}),
     TA::Transition(TA::Location{"CAMERA-ON"},
                    "switch-off",
                    TA::Location{"CAMERA-OFF"},
                    {{"c-camera", AtomicClockConstraintT<std::greater_equal<automata::Time>>{1}},
                     {"c-camera", AtomicClockConstraintT<std::less_equal<automata::Time>>{4}}},
                    {"c-camera"})});
	const auto       product = automata::ta::get_product<std::string, std::string>({robot, camera});
	const MTLFormula pick{AP{"pick"}};
	const MTLFormula put{AP{"put"}};
	const MTLFormula camera_on{AP{"switch-on"}};
	const MTLFormula camera_off{AP{"switch-off"}};
	const auto spec = (!camera_on).until(pick) || finally(camera_off && (!camera_on).until(pick))
	                  || finally(camera_on && finally(pick, logic::TimeInterval(0, 1)))
	                  || (!camera_on).until(put) || finally(camera_off && (!camera_on).until(put))
	                  || finally(camera_on && finally(put, logic::TimeInterval(0, 1)));
	std::set<AP> action_aps;
	for (const auto &a : robot_actions) {
		action_aps.emplace(a);
	}
	for (const auto &a : camera_actions) {
		action_aps.emplace(a);
	}
	auto               ata = mtl_ata_translation::translate(spec, action_aps);
	const unsigned int K   = std::max(product.get_largest_constant(), spec.get_largest_constant());

	std::size_t tree_size        = 0;
	std::size_t pruned_tree_size = 0;
	std::size_t controller_size  = 0;
	std::size_t plant_size       = 0;

	std::unique_ptr<search::Heuristic<long, Search::Node>> heuristic;

	for (auto _ : state) {
		if (weighted) {
			heuristic = generate_heuristic<Search::Node>(state.range(0),
			                                             state.range(1),
			                                             robot_actions,
			                                             state.range(2));
		} else {
			if (state.range(0) == 0) {
				heuristic = std::make_unique<search::BfsHeuristic<long, Search::Node>>();
			} else if (state.range(0) == 1) {
				heuristic = std::make_unique<search::DfsHeuristic<long, Search::Node>>();
			} else if (state.range(0) == 2) {
				heuristic = std::make_unique<search::NumCanonicalWordsHeuristic<long, Search::Node>>();
			} else if (state.range(0) == 3) {
				heuristic = std::make_unique<
				  search::PreferEnvironmentActionHeuristic<long, Search::Node, std::string>>(robot_actions);
			} else if (state.range(0) == 4) {
				heuristic = std::make_unique<search::TimeHeuristic<long, Search::Node>>();
			} else if (state.range(0) == 5) {
				heuristic = std::make_unique<search::RandomHeuristic<long, Search::Node>>(time(NULL));
			} else {
				throw std::invalid_argument("Unexpected argument");
			}
		}
		Search search(
		  &product, &ata, camera_actions, robot_actions, K, true, true, std::move(heuristic));
		search.build_tree(multi_threaded);
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
		plant_size += product.get_locations().size();
		auto controller =
		  controller_synthesis::create_controller(search.get_root(), camera_actions, robot_actions, K);
		controller_size += controller.get_locations().size();
	}
	state.counters["tree_size"] =
	  benchmark::Counter(static_cast<double>(tree_size), benchmark::Counter::kAvgIterations);
	state.counters["pruned_tree_size"] =
	  benchmark::Counter(static_cast<double>(pruned_tree_size), benchmark::Counter::kAvgIterations);
	state.counters["controller_size"] =
	  benchmark::Counter(static_cast<double>(controller_size), benchmark::Counter::kAvgIterations);
	state.counters["plant_size"] =
	  benchmark::Counter(static_cast<double>(plant_size), benchmark::Counter::kAvgIterations);
}

BENCHMARK_CAPTURE(BM_Robot, single_heuristic, false)
  ->DenseRange(0, 5, 1)
  ->MeasureProcessCPUTime()
  ->UseRealTime();
BENCHMARK_CAPTURE(BM_Robot, single_heuristic_single_thread, false, false)
  ->DenseRange(0, 5, 1)
  ->MeasureProcessCPUTime()
  ->UseRealTime();
// Single-threaded with weighted heuristics.
BENCHMARK_CAPTURE(BM_Robot, weighted_single_thread, true, false)
  ->Args({16, 4, 1})
  ->MeasureProcessCPUTime()
  ->UseRealTime();

#ifdef BUILD_LARGE_BENCHMARKS
BENCHMARK_CAPTURE(BM_Robot, weighted, true)
  ->ArgsProduct({benchmark::CreateRange(1, 16, 2),
                 benchmark::CreateRange(1, 16, 2),
                 benchmark::CreateDenseRange(0, 2, 1)})
  ->MeasureProcessCPUTime()
  ->UseRealTime();
#endif
