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
#include "search_tree.h"
#include "synchronous_product.h"

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
	 * @param K The maximal constant occurring in a clock constraint
	 */
	TreeSearch(automata::ta::TimedAutomaton<Location, ActionType> *                            ta,
	           automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ActionType>,
	                                                    logic::AtomicProposition<ActionType>> *ata,
	           RegionIndex                                                                     K)
	: ta_(ta),
	  ata_(ata),
	  tree_root_(std::make_unique<Node>(
	    get_canonical_word(ta->get_initial_configuration(), ata->get_initial_configuration(), K))),
	  K_(K)
	{
		queue_.push(tree_root_.get());
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
		const auto candidate = get_candidate(node->word);
		return ta_->is_accepting_configuration(candidate.first)
		       && ata_->is_accepting_configuration(candidate.second);
	}

	/** Check if there is an ancestor that monotonally dominates the given node
	 * @param node The node to check
	 */
	bool
	is_monotonically_dominated(__attribute__((unused)) Node *node) const
	{
		// TODO implement
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
		if (is_bad_node(current)) {
			current->state = NodeState::BAD;
			return true;
		}
		if (is_monotonically_dominated(current)) {
			current->state = NodeState::GOOD;
			return true;
		}
		assert(current->children.empty());
		for (const auto &word : get_next_canonical_words(*ta_, *ata_, current->word, K_)) {
			auto child = std::make_unique<Node>(word, current);
			queue_.push(child.get());
			current->children.push_back(std::move(child));
		}
		if (current->children.empty()) {
			current->state = NodeState::DEAD;
		}
		return true;
	}

private:
	automata::ta::TimedAutomaton<Location, ActionType> *                            ta_;
	automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ActionType>,
	                                         logic::AtomicProposition<ActionType>> *ata_;

	std::unique_ptr<Node> tree_root_;
	std::queue<Node *>    queue_;
	RegionIndex           K_;
};

} // namespace synchronous_product
