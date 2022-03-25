/***************************************************************************
 *  heuristics.h - Heuristics to evaluate search tree nodes
 *
 *  Created:   Tue 23 Mar 09:05:55 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#ifndef SRC_SYNCHRONOUS_PRODUCT_INCLUDE_SYNCHRONOUS_PRODUCT_HEURISTICS_H
#define SRC_SYNCHRONOUS_PRODUCT_INCLUDE_SYNCHRONOUS_PRODUCT_HEURISTICS_H

#include "search_tree.h"

#include <limits>
#include <random>

namespace tacos::search {

/** The heuristics interface.
 * @tparam ValueT The value type of the heuristic function
 * @tparam LocationT The type of the location of an automaton
 * @tparam ActionT The type of an action of an automaton
 */
template <typename ValueT, typename NodeT>
class Heuristic
{
public:
	/** @brief Compute the cost of the given node.
	 * The higher the cost, the lower the priority.
	 * @param node The node to compute the cost for
	 * @return The cost of the node
	 */
	virtual ValueT compute_cost(NodeT *node) = 0;
	/** Virtual destructor. */
	virtual ~Heuristic()
	{
	}
};

/** @brief The BFS heuristic.
 * The BFS heuristic simply increases the cost with every evaluated node and therefore
 * processes them just like a FIFO queue, resulting in breadth-first sarch.
 * @tparam ValueT The value type of the heuristic function
 * @tparam LocationT The type of the location of an automaton
 * @tparam ActionT The type of an action of an automaton
 */
template <typename ValueT, typename NodeT>
class BfsHeuristic : public Heuristic<ValueT, NodeT>
{
public:
	/** @brief Compute the cost of the given node.
	 * The cost will strictly monotonically increase for each node, thereby emulating breadth-first
	 * search.
	 * @return The cost of the node
	 */
	ValueT
	compute_cost(NodeT *) override
	{
		return ++node_counter;
	}

private:
	std::atomic_size_t node_counter{0};
};

/** @brief The DFS heuristic.
 * The BFS heuristic simply decreases the cost with every evaluated node and therefore
 * processes them just like a LIFO queue, resulting in depth-first sarch.
 */
template <typename ValueT, typename NodeT>
class DfsHeuristic : public Heuristic<ValueT, NodeT>
{
public:
	/** @brief Compute the cost of the given node.
	 * The cost will strictly monotonically increase for each node, thereby emulating breadth-first
	 * search.
	 * @return The cost of the node
	 */
	ValueT
	compute_cost(NodeT *) override
	{
		return -(++node_counter);
	}

private:
	std::atomic_size_t node_counter{0};
};

/** @brief The Time heuristic, which prefers early actions.
 * This heuristic computes the accumulated time from the root node to the current node and
 * prioritizes nodes that occur early.
 * */
template <typename ValueT, typename NodeT>
class TimeHeuristic : public Heuristic<ValueT, NodeT>
{
public:
	/** @brief Compute the cost of the given node.
	 * The cost is the the minimal number of region increments it takes to reach the node.
	 * @param node The node to compute the cost for
	 * @return The cost of the node
	 */
	ValueT
	compute_cost(NodeT *node) override
	{
		return node->min_total_region_increments;
	}
};

/** @brief Prefer environment actions over controller actions.
 * This heuristic assigns a cost of 0 to every node that has at least one environment action as
 * incoming action. Otherwise, it assigns the cost 1.
 */
template <typename ValueT, typename NodeT, typename ActionT>
class PreferEnvironmentActionHeuristic : public Heuristic<ValueT, NodeT>
{
public:
	/** Initialize the heuristic.
	 * @param environment_actions The environment actions that may occur
	 */
	PreferEnvironmentActionHeuristic(const std::set<ActionT> &environment_actions)
	: environment_actions(environment_actions)
	{
	}

	/** Compute the cost of a node.
	 * @param node The node to compute the cost for
	 * @return 0 if the node contains an environment action as incoming action, 1 otherwise.
	 */
	ValueT
	compute_cost(NodeT *node) override
	{
		for (const auto &parent : node->parents) {
			for (const auto &[timed_action, child] : parent->get_children()) {
				if (child.get() == node
				    && environment_actions.find(timed_action.second) != std::end(environment_actions)) {
					return 0;
				}
			}
		}
		return 1;
	}

private:
	std::set<ActionT> environment_actions;
};

/** @brief Prefer nodes with a low number of canonical words. */
template <typename ValueT, typename NodeT>
class NumCanonicalWordsHeuristic : public Heuristic<ValueT, NodeT>
{
public:
	/** Compute the cost of a node.
	 * @param node The node to compute the cost for
	 * @return The number of canonical worrds in the node
	 */
	ValueT
	compute_cost(NodeT *node) override
	{
		return node->words.size();
	}
};

/** @brief Compose multiple heuristics.
 * This heuristic computes a weighted sum over a set of heuristics.
 */
template <typename ValueT, typename NodeT>
class CompositeHeuristic : public Heuristic<ValueT, NodeT>
{
public:
	/** Initialize the heuristic.
	 * @param heuristics A set of pairs (weight, heuristic) to use for the weighted sum
	 */
	CompositeHeuristic(
	  std::vector<std::pair<ValueT, std::unique_ptr<Heuristic<ValueT, NodeT>>>> heuristics)
	: heuristics(std::move(heuristics))
	{
	}

	/** Compute the cost of a node.
	 * @param node The node to compute the cost for
	 * @return The weighted sum over all the heuristics
	 */
	ValueT
	compute_cost(NodeT *node) override
	{
		ValueT res = 0;
		for (auto &&[weight, heuristic] : heuristics) {
			res += weight * heuristic->compute_cost(node);
		}
		return res;
	}

private:
	std::vector<std::pair<ValueT, std::unique_ptr<Heuristic<ValueT, NodeT>>>> heuristics;
};

/** @brief Random heuristic that assigns random costs to nodes.
 */
template <typename ValueT, typename NodeT>
class RandomHeuristic : public Heuristic<ValueT, NodeT>
{
public:
	/** Constructor.
	 * @param seed The seed to use for the random number generator
	 */
	RandomHeuristic(int seed) : seed(seed), random_generator(seed)
	{
	}

	RandomHeuristic() : RandomHeuristic(std::mt19937::default_seed)
	{
	}

	ValueT
	compute_cost(NodeT *) override
	{
		return dist(random_generator);
	}

	/** Get the seed used for the random number generator. */
	int
	get_seed() const
	{
		return seed;
	}

private:
	const int                             seed;
	std::mt19937                          random_generator;
	std::uniform_int_distribution<ValueT> dist;
};

} // namespace tacos::search

#endif /* ifndef SRC_SYNCHRONOUS_PRODUCT_INCLUDE_SYNCHRONOUS_PRODUCT_HEURISTICS_H */
