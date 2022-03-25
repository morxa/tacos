/***************************************************************************
 *  test_fisher.cpp - Fishers mutual exclusion protocol
 *
 *  Created:   Wed  21 Apr 18:07:27 CET 2021
 *  Copyright  2021  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_product.h"
#include "fischer.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "search/create_controller.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "visualization/ta_to_graphviz.h"
#include "visualization/tree_to_graphviz.h"

#include <catch2/catch_test_macros.hpp>
#include <string>

namespace {

using namespace tacos;

using Location   = automata::ta::Location<std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;
using automata::Time;
using F  = logic::MTLFormula<std::string>;
using AP = logic::AtomicProposition<std::string>;
using search::NodeLabel;
using TreeSearch = search::TreeSearch<std::vector<std::string>, std::string>;

TEST_CASE("Two processes", "[.large][fisher]")
{
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%t %v");
	std::size_t               process_count = 2;
	automata::ta::RegionIndex K             = 1;
	const auto &[product, controller_actions, environment_actions] =
	  create_fischer_instance(process_count, 1, 1);
	std::set<AP> actions;
	std::set_union(std::begin(controller_actions),
	               std::end(controller_actions),
	               std::begin(environment_actions),
	               std::end(environment_actions),
	               std::inserter(actions, end(actions)));

	// Specification (good behavior) Forall i: enter_i -> ! enter_j U zero_set_i
	std::vector<logic::MTLFormula<std::string>> phi_i;
	for (std::size_t i = 1; i <= process_count; ++i) {
		for (std::size_t j = 1; j <= process_count; ++j) {
			if (i != j) {
				phi_i.emplace_back(
				  (!AP("enter_" + std::to_string(i)))
				  || !(logic::MTLFormula<std::string>(AP("enter_" + std::to_string(j)))
				         .until(logic::MTLFormula<std::string>(AP("zero_var_" + std::to_string(i))))));
			}
		}
	}
	auto safety   = globally(logic::MTLFormula<std::string>::create_conjunction(phi_i));
	auto liveness = finally(logic::MTLFormula<std::string>(AP("enter_1")))
	                && finally(logic::MTLFormula<std::string>(AP("enter_2")));
	auto good_behavior = safety && liveness;

	auto ata = mtl_ata_translation::translate(!good_behavior, actions);
	INFO("TA: " << product);
	INFO("ATA: " << ata);
	TreeSearch search{&product, &ata, controller_actions, environment_actions, K, true, true};
	search.build_tree(true);
	INFO("Tree:\n" << search::node_to_string(*search.get_root(), true));
#ifdef HAVE_VISUALIZATION
	tacos::visualization::search_tree_to_graphviz(*search.get_root(), true)
	  .render_to_file("fischer2.svg");
	tacos::visualization::ta_to_graphviz(product).render_to_file("fischer2_ta.svg");
	tacos::visualization::ta_to_graphviz(controller_synthesis::create_controller(search.get_root(),
	                                                                             controller_actions,
	                                                                             environment_actions,
	                                                                             K))
	  .render_to_file("fischer2_controller.svg");
#endif
	CHECK(search.get_root()->label == NodeLabel::TOP);
}

} // namespace
