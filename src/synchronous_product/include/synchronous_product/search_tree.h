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
};

/** A node in the search tree
 * @see TreeSearch */
template <typename Location, typename ActionType>
struct SearchTreeNode
{
	/** Construct a node.
	 * @param word The CanonicalABWord of the node
	 */
	SearchTreeNode(const CanonicalABWord<Location, ActionType> &word) : word(word)
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
