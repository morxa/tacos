/***************************************************************************
 *  create_controller.h - Create a controller from a search tree
 *
 *  Created:   Tue 06 Apr 17:09:55 CEST 2021
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

#pragma once

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_regions.h"
#include "search_tree.h"
#include "synchronous_product/canonical_word.h"
#include "synchronous_product/preorder_traversal.h"
#include "synchronous_product/synchronous_product.h"

#include <spdlog/spdlog.h>

#include <stdexcept>
namespace controller_synthesis {

namespace details {
/** Construct a set of constraints from a time successor CanonicalABWord.
 */
template <typename LocationT, typename ActionT>
std::multimap<std::string, automata::ClockConstraint>
get_constraints_from_time_successor(
  const synchronous_product::CanonicalABWord<LocationT, ActionT> &word,
  synchronous_product::RegionIndex                                max_constant,
  automata::ta::ConstraintBoundType                               bound_type)
{
	using TARegionState = synchronous_product::TARegionState<LocationT>;
	std::multimap<std::string, automata::ClockConstraint> res;
	for (const auto &symbol : word) {
		for (const auto &region_state : symbol) {
			assert(std::holds_alternative<TARegionState>(region_state));
			const TARegionState state = std::get<TARegionState>(region_state);
			for (const auto &constraint : automata::ta::get_clock_constraints_from_region_index(
			       state.region_index, 2 * max_constant + 1, bound_type)) {
				res.insert({{state.clock, constraint}});
			}
		}
	}
	return res;
}

template <typename LocationT, typename ActionT>
void
add_node_to_controller(
  const synchronous_product::SearchTreeNode<LocationT, ActionT> *const node,
  synchronous_product::RegionIndex                                     K,
  automata::ta::TimedAutomaton<std::set<synchronous_product::CanonicalABWord<LocationT, ActionT>>,
                               ActionT> *                              controller)
{
	using synchronous_product::NodeLabel;
	using Transition =
	  automata::ta::Transition<std::set<synchronous_product::CanonicalABWord<LocationT, ActionT>>,
	                           ActionT>;
	using Location =
	  automata::ta::Location<std::set<synchronous_product::CanonicalABWord<LocationT, ActionT>>>;
	if (node->label != NodeLabel::TOP) {
		throw std::invalid_argument(
		  "Cannot create a controller for a node that is not labeled with TOP");
	}
	for (const auto &successor : node->children) {
		if (successor->label != NodeLabel::TOP) {
			continue;
		}
		controller->add_location(Location{successor->words});
		controller->add_final_location(Location{successor->words});
		std::map<ActionT, std::set<synchronous_product::RegionIndex>> good_actions;
		for (const auto &[region_increment, action] : successor->incoming_actions) {
			good_actions[action].insert(region_increment);
			controller->add_action(action);
		}
		const auto node_reg_a = reg_a(*std::begin(node->words));
		for (const auto &[action, increments] : good_actions) {
			assert(!increments.empty());
			auto first_good_increment = std::begin(increments);
			for (auto increment = std::begin(increments); increment != std::end(increments);
			     ++increment) {
				if (std::next(increment) == std::end(increments)
				    || *std::next(increment) > *increment + 1) {
					std::multimap<std::string, automata::ClockConstraint> constraints;
					constraints.merge(get_constraints_from_time_successor(
					  synchronous_product::get_nth_time_successor(node_reg_a, *first_good_increment, K),
					  K,
					  automata::ta::ConstraintBoundType::LOWER));
					constraints.merge(get_constraints_from_time_successor(
					  synchronous_product::get_nth_time_successor(node_reg_a, *increment, K),
					  K,
					  automata::ta::ConstraintBoundType::UPPER));

					for (const auto &[clock_name, constraint] : constraints) {
						controller->add_clock(clock_name);
					}
					controller->add_transition(
					  Transition{Location{node->words}, action, Location{successor->words}, constraints, {}});

					first_good_increment = std::next(increment);
				}
			}
		}
		add_node_to_controller(successor.get(), K, controller);
	}
}

} // namespace details

template <typename LocationT, typename ActionT>
automata::ta::TimedAutomaton<std::set<synchronous_product::CanonicalABWord<LocationT, ActionT>>,
                             ActionT>
create_controller(const synchronous_product::SearchTreeNode<LocationT, ActionT> *const root,
                  synchronous_product::RegionIndex                                     K)
{
	using namespace details;
	using synchronous_product::NodeLabel;
	using Location =
	  automata::ta::Location<std::set<synchronous_product::CanonicalABWord<LocationT, ActionT>>>;
	automata::ta::TimedAutomaton<std::set<synchronous_product::CanonicalABWord<LocationT, ActionT>>,
	                             ActionT>
	  controller{{}, Location{root->words}, {}};
	add_node_to_controller(root, K, &controller);
	return controller;
}

} // namespace controller_synthesis
