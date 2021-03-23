/***************************************************************************
 *  heuristics.h - Heuristics to evaluate search tree nodes
 *
 *  Created:   Tue 23 Mar 09:05:55 CET 2021
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

#ifndef SRC_SYNCHRONOUS_PRODUCT_INCLUDE_SYNCHRONOUS_PRODUCT_HEURISTICS_H
#define SRC_SYNCHRONOUS_PRODUCT_INCLUDE_SYNCHRONOUS_PRODUCT_HEURISTICS_H

#include "search_tree.h"

namespace synchronous_product {

/** The heuristics interface.
 * @tparam ValueT The value type of the heuristic function
 * @tparam LocationT The type of the location of an automaton
 * @tparam ActionT The type of an action of an automaton
 */
template <typename ValueT, typename LocationT, typename ActionT>
class Heuristic
{
public:
	virtual ValueT rank(SearchTreeNode<LocationT, ActionT> *node) = 0;
};

/** The BFS heuristic.
 * The BFS heuristic simply decrements the priority with every evaluated node and therefore
 * processes them just like a FIFO queue, resulting in breadth-first sarch.
 * @tparam ValueT The value type of the heuristic function
 * @tparam LocationT The type of the location of an automaton
 * @tparam ActionT The type of an action of an automaton
 */
template <typename ValueT, typename LocationT, typename ActionT>
class BfsHeuristic : public Heuristic<ValueT, LocationT, ActionT>
{
public:
	ValueT
	rank(SearchTreeNode<LocationT, ActionT> *) override
	{
		return -(++node_counter);
	}

private:
	std::atomic_size_t node_counter{0};
};

} // namespace synchronous_product

#endif /* ifndef SRC_SYNCHRONOUS_PRODUCT_INCLUDE_SYNCHRONOUS_PRODUCT_HEURISTICS_H */
