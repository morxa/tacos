/***************************************************************************
 *  search_tree.h - Search tree data structure for the AB search tree
 *
 *  Created:   Mon  1 Feb 15:58:24 CET 2021
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

#include "synchronous_product.h"

#include <memory>

namespace synchronous_product {

/** The state of the search node */
enum class NodeState {
	UNKNOWN, /**< The node is not explored yet */
	GOOD,    /**< No undesired behavior is possible */
	BAD,     /**< Undesired behavior, i.e., the specification is violated */
	DEAD,    /**< The node does not have any successors */
};

/** The label of the search node */
enum class NodeLabel {
	UNLABELED,
	BOTTOM,
	TOP,
};

/** A node in the search tree
 * @see TreeSearch */
template <typename Location, typename ActionType>
struct SearchTreeNode
{
	/** Construct a node.
	 * @param words The CanonicalABWords of the node (being of the same reg_a class)
	 * @param parent The parent of this node, nullptr is this is the root
	 * @param incoming_actions How this node is reachable from its parent
	 */
	SearchTreeNode(const std::set<CanonicalABWord<Location, ActionType>> &words,
	               SearchTreeNode *                                       parent           = nullptr,
	               const std::set<ActionType> &                           incoming_actions = {})
	: words(words), parent(parent), incoming_actions(incoming_actions)
	{
		assert(std::all_of(std::begin(words), std::end(words), [&words](const auto &word) {
			return words.empty() || reg_a(*std::begin(words)) == reg_a(word);
		}));
		// Only the root node has no parent and has no incoming actions.
		assert(parent != nullptr || incoming_actions.empty());
		assert(!incoming_actions.empty() || parent == nullptr);
	}
	/** The words of the node */
	std::set<CanonicalABWord<Location, ActionType>> words;
	/** The state of the node */
	NodeState state = NodeState::UNKNOWN;
	/** Whether we have a successful strategy in the node */
	NodeLabel label = NodeLabel::UNLABELED;
	/** The parent of the node, this node was directly reached from the parent */
	SearchTreeNode *parent = nullptr;
	/** A list of the children of the node, which are reachable by a single transition */
	// TODO change container with custom comparator to set to avoid duplicates (also better
	// performance)
	std::vector<std::unique_ptr<SearchTreeNode>> children = {};
	/** The set of actions on the incoming edge, i.e., how we can reach this node from its parent */
	std::set<ActionType> incoming_actions;
};

} // namespace synchronous_product

template <typename Location, typename ActionType>
void
print_to_ostream(std::ostream &                                                   os,
                 const synchronous_product::SearchTreeNode<Location, ActionType> &node,
                 unsigned int                                                     indent = 0)
{
	using synchronous_product::NodeState;
	for (unsigned int i = 0; i < indent; i++) {
		os << " ";
	}
	os << "-> { ";
	// TODO This should be a separate operator
	for (const auto &action : node.incoming_actions) {
		os << action << " ";
	}
	os << "}: ";
	os << node.words << ": ";
	switch (node.state) {
	case NodeState::UNKNOWN: os << "UNKNOWN"; break;
	case NodeState::GOOD: os << "GOOD"; break;
	case NodeState::BAD: os << "BAD"; break;
	case NodeState::DEAD: os << "DEAD"; break;
	}
	os << '\n';
	for (const auto &child : node.children) {
		print_to_ostream(os, *child, indent + 2);
	}
}

/** Print a node
 * @param os The ostream to print to
 * @param node The node to print
 * @return A reference to the ostream
 */
template <typename Location, typename ActionType>
std::ostream &
operator<<(std::ostream &os, const synchronous_product::SearchTreeNode<Location, ActionType> &node)
{
	print_to_ostream(os, node);
	return os;
}

/** Print a vector of node pointers
 * @param os The ostream to print to
 * @param nodes The node pointers to print
 * @return A reference to the ostream
 */
template <typename Location, typename ActionType>
std::ostream &
operator<<(
  std::ostream &os,
  const std::vector<std::unique_ptr<synchronous_product::SearchTreeNode<Location, ActionType>>>
    &nodes)
{
	for (const auto &node : nodes) {
		os << *node;
	}
	return os;
}
