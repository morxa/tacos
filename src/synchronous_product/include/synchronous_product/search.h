/***************************************************************************
 *  search.h - Construct the search tree over ABConfigurations
 *
 *  Created:   Mon  1 Feb 16:21:53 CET 2021
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

#include "automata/ata.h"
#include "automata/ta.h"
#include "mtl/MTLFormula.h"
#include "operators.h"
#include "reg_a.h"
#include "search_tree.h"
#include "synchronous_product.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <queue>

namespace synchronous_product {

/** Search the configuration tree for a valid controller. */
template <typename Location, typename ActionType>
class TreeSearch
{
	using Node = SearchTreeNode<Location, ActionType>;

public:
	/** Initialize the search.
	 * @param ta The plant to be controlled
	 * @param ata The specification of undesired behaviors
	 * @param controller_actions The actions that the controller may decide to take
	 * @param environment_actions The actions controlled by the environment
	 * @param K The maximal constant occurring in a clock constraint
	 */
	TreeSearch(automata::ta::TimedAutomaton<Location, ActionType> *                            ta,
	           automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ActionType>,
	                                                    logic::AtomicProposition<ActionType>> *ata,
	           std::set<ActionType> controller_actions,
	           std::set<ActionType> environment_actions,
	           RegionIndex          K)
	: ta_(ta),
	  ata_(ata),
	  controller_actions_(controller_actions),
	  environment_actions_(environment_actions),
	  K_(K),
	  tree_root_(std::make_unique<Node>(std::set<CanonicalABWord<Location, ActionType>>{
	    get_canonical_word(ta->get_initial_configuration(), ata->get_initial_configuration(), K)}))
	{
		// Assert that the two action sets are disjoint.
		assert(
		  std::all_of(controller_actions_.begin(), controller_actions_.end(), [this](const auto &a) {
			  return environment_actions_.find(a) == environment_actions_.end();
		  }));
		assert(
		  std::all_of(environment_actions_.begin(), environment_actions_.end(), [this](const auto &a) {
			  return controller_actions_.find(a) == controller_actions_.end();
		  }));

		queue_.push(tree_root_.get());
		assert(queue_.size() == 1);
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

	/** Check if a node is bad, i.e., if it violates the specification@
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

	/** Check if there is an ancestor that monotonally dominates the given node
	 * @param node The node to check
	 */
	bool
	is_monotonically_dominated_by_ancestor(Node *node) const
	{
		const Node *ancestor = node->parent;
		while (ancestor != nullptr) {
			if (is_monotonically_dominated(node->words, ancestor->words)) {
				return true;
			}
			ancestor = ancestor->parent;
		}
		return false;
	}

	/** Compute the next iteration by taking the first item of the queue and expanding it.
	 * @return true if there was still an unexpanded node
	 */
	bool
	step()
	{
		if (queue_.empty()) {
			return false;
		}
		Node *current = queue_.front();
		queue_.pop();
		std::cout << "Processing " << current << ": " << *current;
		if (is_bad_node(current)) {
			current->state = NodeState::BAD;
			return true;
		}
		if (is_monotonically_dominated_by_ancestor(current)) {
			current->state = NodeState::GOOD;
			return true;
		}
		assert(current->children.empty());
		// Represent a set of configurations by their reg_a component so we can later partition the set
		std::map<CanonicalABWord<Location, ActionType>, std::set<CanonicalABWord<Location, ActionType>>>
		  child_classes;
		// Store with which actions we reach each CanonicalABWord
		std::map<CanonicalABWord<Location, ActionType>, std::set<ActionType>> outgoing_actions;
		for (const auto &word : current->words) {
			std::cout << "  Word " << word << '\n';
			for (auto &&[symbol, next_word] : get_next_canonical_words(*ta_, *ata_, word, K_)) {
				// auto child = std::make_unique<Node>(word, current, next_word.first);
				// std::cout << "New child " << child.get() << ": " << *child;
				const auto word_reg = reg_a(next_word);
				child_classes[word_reg].insert(std::move(next_word));
				outgoing_actions[word_reg].insert(symbol);
				// queue_.push(current->children.back().get());
			}
		}
		assert(child_classes.size() == outgoing_actions.size());
		// Create child nodes, where each child contains all successors words of
		// the same reg_a class.
		std::transform(std::make_move_iterator(std::begin(child_classes)),
		               std::make_move_iterator(std::end(child_classes)),
		               std::back_inserter(current->children),
		               [this, current, &outgoing_actions](auto &&map_entry) {
			               auto child =
			                 std::make_unique<Node>(std::move(map_entry.second),
			                                        current,
			                                        std::move(outgoing_actions[map_entry.first]));
			               queue_.push(child.get());
			               return child;
		               });
		if (current->children.empty()) {
			current->state = NodeState::DEAD;
		}
		return true;
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
		if (node->state == NodeState::GOOD || node->state == NodeState::DEAD) {
			node->label = NodeLabel::TOP;
		} else if (node->state == NodeState::BAD) {
			node->label = NodeLabel::BOTTOM;
		} else {
			for (const auto &child : node->children) {
				label(child.get());
			}
			if (std::all_of(std::begin(node->children),
			                std::end(node->children),
			                [this, node](const auto &child) {
				                // The child is either labeled with TOP or it is only reachable with
				                // controller actions (in which case we can choose to ignore it).
				                return child->label == NodeLabel::TOP
				                       || std::includes(std::begin(controller_actions_),
				                                        std::end(controller_actions_),
				                                        std::begin(node->incoming_actions),
				                                        std::end(node->incoming_actions));
			                })) {
				node->label = NodeLabel::TOP;
			} else {
				node->label = NodeLabel::BOTTOM;
			}
		}
	}

private:
	automata::ta::TimedAutomaton<Location, ActionType> *                            ta_;
	automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ActionType>,
	                                         logic::AtomicProposition<ActionType>> *ata_;

	const std::set<ActionType> controller_actions_;
	const std::set<ActionType> environment_actions_;
	RegionIndex                K_;

	std::unique_ptr<Node> tree_root_;
	std::queue<Node *>    queue_;
};

} // namespace synchronous_product
