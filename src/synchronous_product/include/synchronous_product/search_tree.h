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

/** A node in the search tree
 * @see TreeSearch */
template <typename Location, typename ActionType>
struct SearchTreeNode
{
	/** Construct a node.
	 * @param word The CanonicalABWord of the node
	 * @param parent The parent of this node, nullptr is this is the root
	 */
	SearchTreeNode(const CanonicalABWord<Location, ActionType> &word,
	               SearchTreeNode *                             parent = nullptr)
	: word(word), parent(parent)
	{
	}
	/** The word of the node */
	CanonicalABWord<Location, ActionType> word;
	/** The state of the node */
	NodeState state = NodeState::UNKNOWN;
	/** The parent of the node, this node was directly reached from the parent */
	SearchTreeNode *parent = nullptr;
	/** A list of the children of the node, which are reachable by a single transition */
	std::vector<std::unique_ptr<SearchTreeNode>> children = {};
};

} // namespace synchronous_product

/** Print a node
 * @param os The ostream to print to
 * @param node The node to print
 * @return A reference to the ostream
 */
template <typename Location, typename ActionType>
std::ostream &
operator<<(std::ostream &os, const synchronous_product::SearchTreeNode<Location, ActionType> &node)
{
	using synchronous_product::NodeState;
	os << node.word << ": ";
	switch (node.state) {
	case NodeState::UNKNOWN: os << "UNKNOWN"; break;
	case NodeState::GOOD: os << "GOOD"; break;
	case NodeState::BAD: os << "BAD"; break;
	case NodeState::DEAD: os << "DEAD"; break;
	}
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
		os << *node << '\n';
	}
	return os;
}
