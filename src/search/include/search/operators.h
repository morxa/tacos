/***************************************************************************
 *  operators.h - Operators for relations between words
 *
 *  Created:   Mon  8 Feb 14:01:53 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

#include "canonical_word.h"

#include <algorithm>

namespace tacos::search {

template <typename LocationT, typename ActionT, typename ConstraintSymbolT>
class SearchTreeNode;

/**
 * @brief Checks if the word w1 is monotonically dominated by w2.
 * The word w1 is monotonically dominated by w2 if each partition of w1 is a subset of a partition
 * in w2, and those partitions in w2 are strictly monotonically increasing.
 *
 * @param w1 The word which may be dominated.
 * @param w2 The potentially dominating word.
 * @return true if w2 dominates w1.
 * @return false otherwise.
 */
template <typename LocationT, typename ConstraintSymbolT>
bool
is_monotonically_dominated(const CanonicalABWord<LocationT, ConstraintSymbolT> &w1,
                           const CanonicalABWord<LocationT, ConstraintSymbolT> &w2)
{
	auto current_w2_it = w2.begin();
	for (const auto &w1_partition : w1) {
		// Find the w2 partition that includes the current w1 partition.
		const auto next_w2_it =
		  std::find_if(current_w2_it, w2.end(), [&w1_partition](const auto &w2_partition) {
			  return std::includes(w2_partition.begin(),
			                       w2_partition.end(),
			                       w1_partition.begin(),
			                       w1_partition.end());
		  });
		if (next_w2_it == w2.end()) {
			return false;
		}
		current_w2_it = std::next(next_w2_it);
	}
	return true;
}

/**
 * @brief Check the powerset order induced by monotonic domination.
 * Checks if each word of the second set monotonically dominates a word from the first set.
 *
 * @param set1 First set of canonical words which is to be dominated.
 * @param set2 Second set of canonical words which should dominate the first set.
 * @return true if set1 < set2, where < is the powerset order induced by monotonic domination
 * @return false otherwise.
 */
template <typename LocationT, typename ConstraintSymbolT>
bool
is_monotonically_dominated(const std::set<CanonicalABWord<LocationT, ConstraintSymbolT>> &set1,
                           const std::set<CanonicalABWord<LocationT, ConstraintSymbolT>> &set2)
{
	return std::all_of(set2.begin(), set2.end(), [&set1](const auto &word2) {
		return std::any_of(set1.begin(), set1.end(), [&word2](const auto &word1) {
			return is_monotonically_dominated(word1, word2);
		});
	});
}

/** @brief Check monotonic domination for a node and its ancestors.
 *
 * Check if the given words monotonically dominate the given node or one of its ancestors. Note that
 * the recursion relies on the fact that there are no loops in the ancestors. This is guaranteed if
 * a node that monotonically dominates some ancestor is never expanded. If this is not the case,
 * this function may overflow the stack by infinite recursion.
 *
 * @param node Check this node and its ancestors whether its words is monotonically dominated
 * @param words The set of words to compare against the node's words
 * @param seen_nodes A vector of nodes that have already been seen; if the current node has already
 * been seen, the check is aborted.
 * @return true if the given node or one of its ancestors is monotonically dominated
 */
template <typename LocationT, typename ActionT, typename ConstraintSymbolT>
bool
ancestor_is_monotonically_dominated(
  const SearchTreeNode<LocationT, ActionT, ConstraintSymbolT> &               node,
  const std::set<CanonicalABWord<LocationT, ConstraintSymbolT>> &             words,
  std::vector<const SearchTreeNode<LocationT, ActionT, ConstraintSymbolT> *> &seen_nodes)
{
	if (std::find(std::begin(seen_nodes), std::end(seen_nodes), &node) != std::end(seen_nodes)) {
		return false;
	}
	seen_nodes.push_back(&node);
	return is_monotonically_dominated(node.words, words)
	       || std::any_of(node.parents.begin(),
	                      node.parents.end(),
	                      [&words, &seen_nodes](const auto &parent) {
		                      return ancestor_is_monotonically_dominated(*parent, words, seen_nodes);
	                      });
	return false;
}

/** Check if there is an ancestor that monotonally dominates the given node
 * @param node The node to check
 */
template <typename LocationT, typename ActionT, typename ConstraintSymbolT>
bool
dominates_ancestor(SearchTreeNode<LocationT, ActionT, ConstraintSymbolT> *node)
{
	std::vector<const SearchTreeNode<LocationT, ActionT, ConstraintSymbolT> *> seen_nodes = {node};
	return std::any_of(node->parents.begin(),
	                   node->parents.end(),
	                   [node, &seen_nodes](const auto &parent) {
		                   return ancestor_is_monotonically_dominated(*parent, node->words, seen_nodes);
	                   });
}

} // namespace tacos::search
