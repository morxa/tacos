/***************************************************************************
 *  search.h - Construct the search tree over ABConfigurations
 *
 *  Created:   Mon  1 Feb 16:21:53 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include "adapter.h"
#include "automata/ata.h"
#include "automata/ta.h"
#include "canonical_word.h"
#include "heuristics.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "operators.h"
#include "reg_a.h"
#include "search_tree.h"
#include "synchronous_product.h"
#include "utilities/priority_thread_pool.h"
#include "utilities/type_traits.h"
#include "utilities/types.h"

#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <queue>
#include <variant>

namespace tacos::search {

/** @brief Check if the node has a satisfiable ATA configuration.
 * If every word in the node contains an ATA sink location, than none of those configurations is
 * satisfiable.
 * @return false if every word contains an ATA sink location
 */
template <typename Location, typename ActionType, typename ConstraintSymbolType>
bool
has_satisfiable_ata_configuration(
  const SearchTreeNode<Location, ActionType, ConstraintSymbolType> &node)
{
	return !std::all_of(std::begin(node.words), std::end(node.words), [](const auto &word) {
		return std::any_of(std::begin(word), std::end(word), [](const auto &component) {
			return std::find_if(
			         std::begin(component),
			         std::end(component),
			         [](const auto &region_symbol) {
				         return std::holds_alternative<ATARegionState<ConstraintSymbolType>>(region_symbol)
				                && std::get<ATARegionState<ConstraintSymbolType>>(region_symbol).formula
				                     == logic::MTLFormula<ConstraintSymbolType>{
				                       mtl_ata_translation::get_sink<ConstraintSymbolType>()};
			         })
			       != std::end(component);
		});
	});
}

namespace details {

template <typename Location, typename ActionType, typename ConstraintSymbolType>
void
label_graph(SearchTreeNode<Location, ActionType, ConstraintSymbolType> *node,
            const std::set<ActionType>                                 &controller_actions,
            const std::set<ActionType>                                 &environment_actions,
            std::set<SearchTreeNode<Location, ActionType, ConstraintSymbolType> *> &visited)
{
	if (node->label != NodeLabel::UNLABELED) {
		return;
	}
	if (std::find(std::begin(visited), std::end(visited), node) != std::end(visited)) {
		// This node was already visited, meaning that we have found a loop. In a loop, there is always
		// a monotonic domination, because monotonic domination is reflexive.
		node->label        = NodeLabel::TOP;
		node->label_reason = LabelReason::MONOTONIC_DOMINATION;
		return;
	}
	visited.insert(node);
	if (node->state == NodeState::GOOD) {
		node->label_reason = LabelReason::GOOD_NODE;
		node->set_label(NodeLabel::TOP);
	} else if (node->state == NodeState::DEAD) {
		node->label_reason = LabelReason::DEAD_NODE;
		node->set_label(NodeLabel::TOP);
	} else if (node->state == NodeState::BAD) {
		node->label_reason = LabelReason::BAD_NODE;
		node->set_label(NodeLabel::BOTTOM);
	} else {
		for (const auto &[action, child] : node->get_children()) {
			if (child.get() != node) {
				label_graph(child.get(), controller_actions, environment_actions, visited);
			}
		}
		bool        has_enviroment_step{false};
		RegionIndex first_good_controller_step{std::numeric_limits<RegionIndex>::max()};
		// RegionIndex first_bad_controller_step{std::numeric_limits<RegionIndex>::max()};
		// RegionIndex first_good_environment_step{std::numeric_limits<RegionIndex>::max()};
		RegionIndex first_bad_environment_step{std::numeric_limits<RegionIndex>::max()};
		for (const auto &[timed_action, child] : node->get_children()) {
			const auto &[step, action] = timed_action;
			if (controller_actions.find(action) != std::end(controller_actions)) {
				assert(environment_actions.find(action) == std::end(environment_actions));
				if (child->label == NodeLabel::TOP) {
					first_good_controller_step = std::min(first_good_controller_step, step);
				} else {
					// first_bad_controller_step = std::min(first_bad_controller_step, step);
				}
			} else {
				assert(environment_actions.find(action) != std::end(environment_actions));
				has_enviroment_step = true;
				if (child->label == NodeLabel::TOP) {
					// first_good_environment_step = std::min(first_good_environment_step, step);
				} else {
					first_bad_environment_step = std::min(first_bad_environment_step, step);
				}
			}
		}
		// Formally, the controller selects a subset of actions U such that
		// (1) U is deterministic: it cannot select the same action twice with different clock resets)
		// (2) U is non-restricting: if there is an environment action at step i, then the controller
		// must select a controller action at step j < i or it must select the environment action (3) U
		// is non-blocking: if there is some successor, then U must not be empty. The environment then
		// selects exactly one element of U.
		if (first_good_controller_step < first_bad_environment_step) {
			// The controller can just select the good controller action.
			node->label_reason = LabelReason::GOOD_CONTROLLER_ACTION_FIRST;
			node->set_label(NodeLabel::TOP);
		} else if (has_enviroment_step
		           && first_bad_environment_step == std::numeric_limits<RegionIndex>::max()) {
			// There is an environment action and no environment action is bad
			// -> the controller can just select all environment actions
			node->label_reason = LabelReason::NO_BAD_ENV_ACTION;
			node->set_label(NodeLabel::TOP);
		} else if (!has_enviroment_step) {
			// All controller actions must be bad (otherwise we would be in the first case)
			// -> no controller strategy
			node->label_reason = LabelReason::ALL_CONTROLLER_ACTIONS_BAD;
			node->set_label(NodeLabel::BOTTOM);
		} else {
			// There must be an environment action (otherwise case 3) and one of them must be bad
			// (otherwise case 2).
			assert(first_bad_environment_step < std::numeric_limits<RegionIndex>::max());
			node->label_reason = LabelReason::BAD_ENV_ACTION_FIRST;
			node->set_label(NodeLabel::BOTTOM);
		}
	}
}
} // namespace details

template <typename Location, typename ActionType, typename ConstraintSymbolType>
void
label_graph(SearchTreeNode<Location, ActionType, ConstraintSymbolType> *node,
            const std::set<ActionType>                                 &controller_actions,
            const std::set<ActionType>                                 &environment_actions)
{
	std::set<SearchTreeNode<Location, ActionType, ConstraintSymbolType> *> visited;
	return details::label_graph(node, controller_actions, environment_actions, visited);
}

/** Search the configuration tree for a valid controller. */
template <typename Location,
          typename ActionType,
          typename ConstraintSymbolType = ActionType,
          bool use_location_constraints = false,
          typename Plant =
            automata::ta::TimedAutomaton<typename Location::UnderlyingType, ActionType>,
          bool use_set_semantics = false>
class TreeSearch
{
public:
	/** The type of the input symbols that the ATA accepts. */
	using ATAInputType =
	  /* if use_set_semantics is true, then the ATASymbolT is the same as
	   * std::set<ConstraintSymbolType>, otherwise it is just ConstraintSymbolType> */
	  typename std::disjunction<
	    utilities::values_equal<use_set_semantics, false, ConstraintSymbolType>,
	    utilities::values_equal<use_set_semantics, true, std::set<ConstraintSymbolType>>,
	    std::void_t<void>>::type;
	/** The corresponding Node type of this search. */
	using Node = SearchTreeNode<Location, ActionType, ConstraintSymbolType>;

	/** Initialize the search.
	 * @param ta The plant to be controlled
	 * @param ata The specification of undesired behaviors
	 * @param controller_actions The actions that the controller may decide to take
	 * @param environment_actions The actions controlled by the environment
	 * @param K The maximal constant occurring in a clock constraint
	 * @param incremental_labeling True, if incremental labeling should be used (default=false)
	 * @param terminate_early If true, cancel the children of a node that has already been labeled
	 * @param heuristic The heuristic to use during tree expansion
	 */
	TreeSearch(
	  // const automata::ta::TimedAutomaton<Location, ActionType> *                                ta,
	  const Plant                                                                      *ta,
	  automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ConstraintSymbolType>,
	                                           logic::AtomicProposition<ATAInputType>> *ata,
	  std::set<ActionType>                   controller_actions,
	  std::set<ActionType>                   environment_actions,
	  RegionIndex                            K,
	  bool                                   incremental_labeling = false,
	  bool                                   terminate_early      = false,
	  std::unique_ptr<Heuristic<long, Node>> heuristic = std::make_unique<BfsHeuristic<long, Node>>())
	: ta_(ta),
	  ata_(ata),
	  controller_actions_(controller_actions),
	  environment_actions_(environment_actions),
	  K_(K),
	  incremental_labeling_(incremental_labeling),
	  terminate_early_(terminate_early),
	  tree_root_(std::make_shared<Node>(
	    std::set<CanonicalABWord<typename Plant::Location, ConstraintSymbolType>>{
	      get_canonical_word(ta->get_initial_configuration(), ata->get_initial_configuration(), K)})),
	  nodes_{{{{}, tree_root_}}},
	  heuristic(std::move(heuristic))
	{
		static_assert(use_location_constraints || std::is_same_v<ActionType, ConstraintSymbolType>);
		// Assert that the two action sets are disjoint.
		assert(
		  std::all_of(controller_actions_.begin(), controller_actions_.end(), [this](const auto &a) {
			  return environment_actions_.find(a) == environment_actions_.end();
		  }));
		assert(
		  std::all_of(environment_actions_.begin(), environment_actions_.end(), [this](const auto &a) {
			  return controller_actions_.find(a) == controller_actions_.end();
		  }));
		tree_root_->min_total_region_increments = 0;
		add_node_to_queue(tree_root_.get());
	}

	/** Get the root of the search tree.
	 * @return A pointer to the root, only valid as long as the TreeSearch object has not been
	 * destroyed
	 */
	Node *
	get_root() const
	{
		return tree_root_.get();
	}

	/** Check if a node is bad, i.e., if it violates the specification.
	 * @param node A pointer to the node to check
	 * @return true if the node is bad
	 */
	bool
	is_bad_node(Node *node) const
	{
		return std::any_of(node->words.begin(), node->words.end(), [this](const auto &word) {
			const auto candidate = get_candidate(word);
			return ta_->is_accepting_configuration(candidate.first)
			       && ata_->is_accepting_configuration(candidate.second);
		});
	}

	/** Add a node the processing queue. This adds a new task to the thread pool that expands the node
	 * asynchronously.
	 * @param node The node to expand */
	void
	add_node_to_queue(Node *node)
	{
		pool_.add_job([this, node] { expand_node(node); }, -heuristic->compute_cost(node));
	}

	/** Build the complete search tree by expanding nodes recursively.
	 * @param multi_threaded If set to true, run the thread pool. Otherwise, process the jobs
	 * synchronously with a single thread. */
	void
	build_tree(bool multi_threaded = true)
	{
		if (multi_threaded) {
			pool_.start();
			pool_.wait();
		} else {
			while (step()) {}
		}
	}

	/** Compute the next iteration by taking the first item of the queue and expanding it.
	 * @return true if there was still an unexpanded node
	 */
	bool
	step()
	{
		utilities::QueueAccess queue_access{&pool_};
		SPDLOG_TRACE("Getting next node from queue, queue size is {}", queue_access.get_size());
		if (queue_access.empty()) {
			return false;
		}
		auto step_function = std::get<1>(queue_access.top());
		queue_access.pop();
		step_function();
		return true;
	}

	/** Process and expand the given node.  */
	void
	expand_node(Node *node)
	{
		if (node->label != NodeLabel::UNLABELED) {
			// The node was already labeled, nothing to do.
			return;
		}
		bool is_expanding = node->is_expanding.exchange(true);
		if (is_expanding) {
			// The node is already being expanded.
			return;
		}
		SPDLOG_TRACE("Processing {}", *node);
		if (is_bad_node(node)) {
			SPDLOG_DEBUG("Node {} is BAD", *node);
			node->label_reason = LabelReason::BAD_NODE;
			node->state        = NodeState::BAD;
			node->is_expanded  = true;
			node->is_expanding = false;
			if (incremental_labeling_) {
				node->set_label(NodeLabel::BOTTOM, terminate_early_);
				node->label_propagate(controller_actions_, environment_actions_, terminate_early_);
			}
			return;
		}
		if (!has_satisfiable_ata_configuration(*node)) {
			node->label_reason = LabelReason::NO_ATA_SUCCESSOR;
			node->state        = NodeState::GOOD;
			node->is_expanded  = true;
			node->is_expanding = false;
			if (incremental_labeling_) {
				node->set_label(NodeLabel::TOP, terminate_early_);
				node->label_propagate(controller_actions_, environment_actions_, terminate_early_);
			}
			return;
		}
		if (dominates_ancestor(node)) {
			node->label_reason = LabelReason::MONOTONIC_DOMINATION;
			node->state        = NodeState::GOOD;
			node->is_expanded  = true;
			node->is_expanding = false;
			if (incremental_labeling_) {
				node->set_label(NodeLabel::TOP, terminate_early_);
				node->label_propagate(controller_actions_, environment_actions_, terminate_early_);
			}
			return;
		}

		std::set<Node *> new_children;
		std::set<Node *> existing_children;
		if (node->get_children().empty()) {
			std::tie(new_children, existing_children) = compute_children(node);
		}

		node->is_expanded  = true;
		node->is_expanding = false;
		if (node->label == NodeLabel::CANCELED) {
			// The node has been canceled in the meantime, do not add children to queue.
			return;
		}
		for (const auto &child : existing_children) {
			SPDLOG_TRACE("Found existing node for {}", fmt::ptr(child));
			if (child->label == NodeLabel::CANCELED) {
				SPDLOG_DEBUG("Expansion of {}: Found existing child {}, is canceled, re-adding",
				             fmt::ptr(node),
				             fmt::ptr(child));
				child->reset_label();
				add_node_to_queue(child);
			}
		}
		if (incremental_labeling_ && !existing_children.empty()) {
			// There is an existing child, directly check the labeling.
			SPDLOG_TRACE("Node {} has existing child, updating labels", node_to_string(*node, false));
			node->label_propagate(controller_actions_, environment_actions_, terminate_early_);
		}
		for (const auto &child : new_children) {
			add_node_to_queue(child);
		}
		SPDLOG_TRACE("Node has {} children, {} of them new",
		             node->get_children().size(),
		             new_children.size());
		if (node->get_children().empty()) {
			node->label_reason = LabelReason::DEAD_NODE;
			node->state        = NodeState::DEAD;
			if (incremental_labeling_) {
				node->set_label(NodeLabel::TOP, terminate_early_);
				node->label_propagate(controller_actions_, environment_actions_, terminate_early_);
			}
		}
	}

	/** Compute the final tree labels.
	 * @param node The node to start the labeling at (e.g., the root of the tree)
	 */
	void
	label(Node *node = nullptr)
	{
		if (node == nullptr) {
			node = get_root();
		}
		return label_graph(node, controller_actions_, environment_actions_);
	}

	/** Get the size of the search graph.
	 * @return The number of nodes in the search graph
	 */
	size_t
	get_size() const
	{
		std::lock_guard lock{nodes_mutex_};
		return nodes_.size();
	}

	/** Get the current search nodes. */
	const std::map<std::set<CanonicalABWord<Location, ConstraintSymbolType>>, std::shared_ptr<Node>> &
	get_nodes()
	{
		return nodes_;
	}

private:
	std::pair<std::set<Node *>, std::set<Node *>>
	compute_children(Node *node)
	{
		if (node == nullptr) {
			return {};
		}
		assert(node->get_children().empty());
		// Represent a set of configurations by their reg_a component so we can later partition the
		// set
		std::map<std::pair<RegionIndex, ActionType>,
		         std::map<CanonicalABWord<Location, ConstraintSymbolType>,
		                  std::set<CanonicalABWord<Location, ConstraintSymbolType>>>>
		  child_classes;

		for (const auto &word : node->words) {
			for (const auto &[increment, time_successor] : get_time_successors(word, K_)) {
				auto successors =
				  get_next_canonical_words<Plant,
				                           ActionType,
				                           ConstraintSymbolType,
				                           use_location_constraints,
				                           use_set_semantics>(controller_actions_, environment_actions_)(
				    *ta_, *ata_, get_candidate(time_successor), increment, K_);
				for (const auto &[symbol, successor] : successors) {
					const auto word_reg = reg_a(successor);
					child_classes[std::make_pair(increment, symbol)][word_reg].insert(successor);
				}
			}
		}

		std::set<Node *> new_children;
		std::set<Node *> existing_children;
		// Create child nodes, where each child contains all successors words of
		// the same reg_a class.
		{
			std::lock_guard lock{nodes_mutex_};
			for (const auto &[timed_action, word_map] : child_classes) {
				for (const auto &[reg_a, words] : word_map) {
					auto [child_it, is_new] = nodes_.insert({words, std::make_shared<Node>(words)});
					const std::shared_ptr<Node> &child_ptr = child_it->second;
					node->add_child(timed_action, child_ptr);
					if (is_new) {
						SPDLOG_TRACE("New child: {}", words);
						new_children.insert(child_ptr.get());
					} else {
						existing_children.insert(child_ptr.get());
					}
				}
			}
		}
		return {new_children, existing_children};
	}

	const Plant *const ta_;
	const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ConstraintSymbolType>,
	                                               logic::AtomicProposition<ATAInputType>>
	  *const ata_;

	const std::set<ActionType> controller_actions_;
	const std::set<ActionType> environment_actions_;
	RegionIndex                K_;
	const bool                 incremental_labeling_;
	const bool                 terminate_early_{false};

	mutable std::mutex    nodes_mutex_;
	std::shared_ptr<Node> tree_root_;
	std::map<std::set<CanonicalABWord<Location, ConstraintSymbolType>>, std::shared_ptr<Node>> nodes_;
	utilities::ThreadPool<long> pool_{utilities::ThreadPool<long>::StartOnInit::NO};
	std::unique_ptr<Heuristic<long, SearchTreeNode<Location, ActionType, ConstraintSymbolType>>>
	  heuristic;
};

} // namespace tacos::search
