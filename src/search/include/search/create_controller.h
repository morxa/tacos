/***************************************************************************
 *  create_controller.h - Create a controller from a search tree
 *
 *  Created:   Tue 06 Apr 17:09:55 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_regions.h"
#include "search/canonical_word.h"
#include "search/synchronous_product.h"
#include "search_tree.h"

#include <spdlog/spdlog.h>

#include <stdexcept>
namespace tacos::controller_synthesis {

namespace details {
/** Construct a set of constraints from a time successor CanonicalABWord.
 */
template <typename LocationT, typename ActionT>
std::multimap<std::string, automata::ClockConstraint>
get_constraints_from_time_successor(const search::CanonicalABWord<LocationT, ActionT> &word,
                                    search::RegionIndex                                max_constant,
                                    automata::ta::ConstraintBoundType                  bound_type)
{
	using TARegionState = search::TARegionState<LocationT>;
	std::multimap<std::string, automata::ClockConstraint> res;
	const auto                                            max_region_index = 2 * max_constant + 1;
	for (const auto &symbol : word) {
		for (const auto &region_state : symbol) {
			assert(std::holds_alternative<TARegionState>(region_state));
			const TARegionState state = std::get<TARegionState>(region_state);
			for (const auto &constraint : automata::ta::get_clock_constraints_from_region_index(
			       state.region_index, max_region_index, bound_type)) {
				res.insert({{state.clock, constraint}});
			}
		}
	}
	return res;
}

/** @brief Compute the corresponding constraints from a set of outgoing actions of a node.
 * Given the word of a node and the node's outgoing actions to a successor, we can compute the clock
 * constraints for that transition from the region increments of the outgoing actions. Do this by
 * computing the time successor corresponding to each region increment, then computing the
 * corresponding constraints, and then post-process the constraints such that neighboring intervals
 * are merged into one constraint.
 * @param canonical_words The canonical words of the node.
 * @param actions The outgoing actions of the node as set of pairs (region increment, action name)
 * @param K The value of the maximal constant occurring anywhere in the input problem
 * @return A multimap, where each entry is a pair (a, c), where c is a multimap of clock constraints
 * necessary when taking action a.
 */
template <typename LocationT, typename ActionT, typename ConstraintSymbolT>
std::multimap<ActionT, std::multimap<std::string, automata::ClockConstraint>>
get_constraints_from_outgoing_action(
  const std::set<search::CanonicalABWord<LocationT, ConstraintSymbolT>> canonical_words,
  const std::pair<search::RegionIndex, ActionT> &                       timed_action,
  search::RegionIndex                                                   K)
{
	std::map<ActionT, std::set<search::RegionIndex>> good_actions;
	// TODO merging of the constraints is broken because we now get only a single action.
	good_actions[timed_action.second].insert(timed_action.first);

	// We only need the reg_a of the words. As we know that they are all the same, we can just take
	// the first one.
	assert(reg_a(*std::begin(canonical_words)) == reg_a(*std::rbegin(canonical_words)));
	const auto node_reg_a = reg_a(*std::begin(canonical_words));

	std::multimap<ActionT, std::multimap<std::string, automata::ClockConstraint>> res;
	for (const auto &[action, increments] : good_actions) {
		assert(!increments.empty());

		auto first_good_increment = std::begin(increments);
		for (auto increment = std::begin(increments); increment != std::end(increments); ++increment) {
			if (std::next(increment) == std::end(increments) || *std::next(increment) > *increment + 1) {
				std::multimap<std::string, automata::ClockConstraint> constraints;
				if (first_good_increment == increment) {
					// They are the same, create both constraints at the same time to obtain a = constraint
					// for even regions. constraints.merge(
					constraints.merge(get_constraints_from_time_successor(
					  search::get_nth_time_successor(node_reg_a, *first_good_increment, K),
					  K,
					  automata::ta::ConstraintBoundType::BOTH));
				} else {
					constraints.merge(get_constraints_from_time_successor(
					  search::get_nth_time_successor(node_reg_a, *first_good_increment, K),
					  K,
					  automata::ta::ConstraintBoundType::LOWER));
					constraints.merge(get_constraints_from_time_successor(
					  search::get_nth_time_successor(node_reg_a, *increment, K),
					  K,
					  automata::ta::ConstraintBoundType::UPPER));
				}
				res.insert(std::make_pair(action, constraints));
				first_good_increment = std::next(increment);
			}
		}
	}
	return res;
}

template <typename LocationT, typename ActionT, typename ConstraintSymbolT>
void
add_node_to_controller(
  const search::SearchTreeNode<LocationT, ActionT, ConstraintSymbolT> *const node,
  std::set<ActionT>                                                          controller_actions,
  std::set<ActionT>                                                          environment_actions,
  search::RegionIndex                                                        K,
  bool                                                                       minimize_controller,
  automata::ta::TimedAutomaton<std::set<search::CanonicalABWord<LocationT, ConstraintSymbolT>>,
                               ActionT> *                                    controller)
{
	using search::NodeLabel;
	using Transition =
	  automata::ta::Transition<std::set<search::CanonicalABWord<LocationT, ConstraintSymbolT>>,
	                           ActionT>;
	using Location =
	  automata::ta::Location<std::set<search::CanonicalABWord<LocationT, ConstraintSymbolT>>>;
	if (node->label != NodeLabel::TOP) {
		throw std::invalid_argument(
		  "Cannot create a controller for a node that is not labeled with TOP");
	}
	assert(std::is_sorted(std::begin(node->get_children()), std::end(node->get_children())));
	for (const auto &[timed_action, successor] : node->get_children()) {
		if (successor->label != NodeLabel::TOP) {
			continue;
		}
		bool new_location = controller->add_location(Location{successor->words});
		controller->add_final_location(Location{successor->words});

		for (const auto &[action, constraints] :
		     get_constraints_from_outgoing_action(node->words, timed_action, K)) {
			for (const auto &[clock, _constraint] : constraints) {
				controller->add_clock(clock);
			}
			controller->add_action(action);
			controller->add_transition(
			  Transition{Location{node->words}, action, Location{successor->words}, constraints, {}});
		}
		if (new_location) {
			// To break circles in the search graph, only add the successor if it is actually a new
			// location.
			add_node_to_controller(successor.get(),
			                       controller_actions,
			                       environment_actions,
			                       K,
			                       minimize_controller,
			                       controller);
		}
		if (minimize_controller
		    && controller_actions.find(timed_action.second) != std::end(controller_actions)) {
			break;
		}
	}
}

} // namespace details

template <typename LocationT, typename ActionT, typename ConstraintSymbolT>
automata::ta::TimedAutomaton<std::set<search::CanonicalABWord<LocationT, ConstraintSymbolT>>,
                             ActionT>
create_controller(const search::SearchTreeNode<LocationT, ActionT, ConstraintSymbolT> *const root,
                  std::set<ActionT>   controller_actions,
                  std::set<ActionT>   environment_actions,
                  search::RegionIndex K,
                  bool                minimize_controller = true)
{
	using namespace details;
	using search::NodeLabel;
	using Location =
	  automata::ta::Location<std::set<search::CanonicalABWord<LocationT, ConstraintSymbolT>>>;
	automata::ta::TimedAutomaton<std::set<search::CanonicalABWord<LocationT, ConstraintSymbolT>>,
	                             ActionT>
	  controller{{}, Location{root->words}, {}};
	add_node_to_controller(
	  root, controller_actions, environment_actions, K, minimize_controller, &controller);
	return controller;
}

} // namespace tacos::controller_synthesis
