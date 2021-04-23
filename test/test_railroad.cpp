/***************************************************************************
 *  test_railroad.cpp - A railroad example
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
#include "railroad.h"
#include "search/create_controller.h"
#include "search/heuristics.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "visualization/ta_to_graphviz.h"
#include "visualization/tree_to_graphviz.h"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

using Location   = automata::ta::Location<std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;
using automata::Time;
using F  = logic::MTLFormula<std::string>;
using AP = logic::AtomicProposition<std::string>;
using search::NodeLabel;
using TreeSearch = search::TreeSearch<std::vector<std::string>, std::string>;

TEST_CASE("A single railroad crossing", "[.railroad]")
{
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%t %v");
	const auto &[product, controller_actions, environment_actions] = create_crossing_problem({2});
	std::set<AP> actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               inserter(actions, end(actions)));
	const auto finish_close = F{AP{"finish_close_1"}};
	const auto start_open   = F{AP{"start_open_1"}};
	const auto finish_open  = F{AP{"finish_open_1"}};
	const auto enter        = F{AP{"enter_1"}};
	const auto leave        = F{AP{"leave_1"}};
	const auto travel       = F{AP{"travel_1"}};
	const auto spec         = enter.dual_until(!finish_close) || start_open.dual_until(!leave)
	                  || travel.dual_until(!finish_open);
	auto ata = mtl_ata_translation::translate(spec, actions);
	INFO("TA: " << product);
	INFO("ATA: " << ata);
	const auto K = std::max(automata::ta::get_maximal_region_index(product),
	                        static_cast<unsigned int>(spec.get_maximal_region_index()));
	TreeSearch search{
	  &product,
	  &ata,
	  controller_actions,
	  environment_actions,
	  K,
	  true,
	  true,
	  std::make_unique<search::TimeHeuristic<long, std::vector<std::string>, std::string>>()};

	search.build_tree(true);
	// INFO("Tree:\n" << search::node_to_string(*search.get_root(), true));
#ifdef HAVE_VISUALIZATION
	visualization::search_tree_to_graphviz(*search.get_root(), true).render_to_file("railroad1.svg");
	visualization::ta_to_graphviz(controller_synthesis::create_controller(search.get_root(), 2))
	  .render_to_file("railroad_controller.pdf");
#endif
	CHECK(search.get_root()->label == NodeLabel::TOP);
}

TEST_CASE("Two railroad crossings", "[.medium][railroad]")
{
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%t %v");
	const auto   problem             = create_crossing_problem({2, 4});
	auto         plant               = std::get<0>(problem);
	auto         controller_actions  = std::get<1>(problem);
	auto         environment_actions = std::get<2>(problem);
	std::set<AP> actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               inserter(actions, end(actions)));
	const auto finish_close_1 = F{AP{"finish_close_1"}};
	const auto start_open_1   = F{AP{"start_open_1"}};
	const auto enter_1        = F{AP{"enter_1"}};
	const auto finish_close_2 = F{AP{"finish_close_2"}};
	const auto start_open_2   = F{AP{"start_open_2"}};
	const auto enter_2        = F{AP{"enter_2"}};
	auto       ata =
	  mtl_ata_translation::translate(((!finish_close_1).until(enter_1) && finally(enter_1))
	                                   || ((!finish_close_2).until(enter_2) && finally(enter_2)),
	                                 actions);
	INFO("TA: " << plant);
	INFO("ATA: " << ata);
	BENCHMARK("Run the search")
	{
		TreeSearch search{
		  &plant,
		  &ata,
		  controller_actions,
		  environment_actions,
		  4,
		  true,
		  true,
		  std::make_unique<search::TimeHeuristic<long, std::vector<std::string>, std::string>>()};
		search.build_tree(true);
		// INFO("Tree:\n" << search::node_to_string(*search.get_root(), true));
#ifdef HAVE_VISUALIZATION
		// visualization::search_tree_to_graphviz(*search.get_root(), true)
		//  .render_to_file("railroad2.svg");
#endif
		std::size_t size         = 0;
		std::size_t non_canceled = 0;
		for (auto it = search::begin(search.get_root()); it != search::end(search.get_root()); it++) {
			size++;
			if (it->label != search::NodeLabel::CANCELED) {
				non_canceled++;
			}
		}
		INFO("Tree size: " << size);
		INFO("Non-canceled: " << non_canceled);
		CHECK(search.get_root()->label == NodeLabel::TOP);
	};
}

TEST_CASE("Three railroad crossings", "[.large][railroad]")
{
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%t %v");
	const auto   problem             = create_crossing_problem({2, 2, 2});
	auto         plant               = std::get<0>(problem);
	auto         controller_actions  = std::get<1>(problem);
	auto         environment_actions = std::get<2>(problem);
	std::set<AP> actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               inserter(actions, end(actions)));
	const auto finish_close_1 = F{AP{"finish_close_1"}};
	const auto start_open_1   = F{AP{"start_open_1"}};
	const auto enter_1        = F{AP{"enter_1"}};
	const auto finish_close_2 = F{AP{"finish_close_2"}};
	const auto start_open_2   = F{AP{"start_open_2"}};
	const auto enter_2        = F{AP{"enter_2"}};
	const auto finish_close_3 = F{AP{"finish_close_3"}};
	const auto start_open_3   = F{AP{"start_open_3"}};
	const auto enter_3        = F{AP{"enter_3"}};
	auto       ata =
	  mtl_ata_translation::translate(((!finish_close_1).until(enter_1) && finally(enter_1))
	                                   || ((!finish_close_2).until(enter_2) && finally(enter_2))
	                                   || ((!finish_close_3).until(enter_3) && finally(enter_3)),
	                                 actions);
	// INFO("TA: " << product);
	// INFO("ATA: " << ata);
	BENCHMARK("Run the search")
	{
		TreeSearch search{
		  &plant,
		  &ata,
		  controller_actions,
		  environment_actions,
		  2,
		  true,
		  true,
		  std::make_unique<search::TimeHeuristic<long, std::vector<std::string>, std::string>>()};
		search.build_tree(true);
#ifdef HAVE_VISUALIZATION
		visualization::search_tree_to_graphviz(*search.get_root(), true)
		  .render_to_file("railroad3.svg");
#endif
		std::size_t size         = 0;
		std::size_t non_canceled = 0;
		for (auto it = search::begin(search.get_root()); it != search::end(search.get_root()); it++) {
			size++;
			if (it->label != search::NodeLabel::CANCELED) {
				non_canceled++;
			}
		}
		INFO("Tree size: " << size);
		INFO("Non-canceled: " << non_canceled);
		CHECK(search.get_root()->label == NodeLabel::TOP);
	};
	// INFO("Tree:\n" << search::node_to_string(*search.get_root(), true));
}

} // namespace
