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

#include "automata/ta_regions.h"
#include "canonical_word.h"
#include "preorder_traversal.h"
#include "reg_a.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <iostream>
#include <limits>
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
	               const std::set<std::pair<RegionIndex, ActionType>> &   incoming_actions = {})
	: words(words), parent(parent), incoming_actions(incoming_actions)
	{
		assert(std::all_of(std::begin(words), std::end(words), [&words](const auto &word) {
			return words.empty() || reg_a(*std::begin(words)) == reg_a(word);
		}));
		// TODO check if we may only store one region index, because they should be the same (?)
		// Only the root node has no parent and has no incoming actions.
		assert(parent != nullptr || incoming_actions.empty());
		assert(!incoming_actions.empty() || parent == nullptr);
	}
	/**
	 * @brief Implements incremental labeling during search, bottom up. Nodes are labelled as soon as
	 * their label state can definitely be determined either because they are leaf-nodes or because
	 * the labeling of child nodes permits to determine a label (see details).
	 * @details Leaf-nodes can directly be labelled (in most cases?), the corresponding label pushed
	 * upwards in the search tree may allow for shortening the search significantly in the following
	 * cases: 1) A child is labelled "BAD" and there is no control-action which can be taken before
	 * that is labelled "GOOD" -> the node can be labelled as "BAD". 2) A child is labelled "GOOD" and
	 * came from a control-action and there is no non-"GOOD" environmental-action happening before ->
	 * the node can be labelled "GOOD". The call should be propagated to the parent node in case the
	 * labelling has been determined.
	 * @param controller_actions The set of controller actions
	 * @param environment_actions The set of environment actions
	 */
	void
	label_propagate(const std::set<ActionType> &controller_actions,
	                const std::set<ActionType> &environment_actions)
	{
		// leaf-nodes should always be labelled directly
		assert(!children.empty() || label != NodeLabel::UNLABELED);
		// if not already happened: call recursively on parent node
		if (children.empty()) {
			assert(label != NodeLabel::UNLABELED);
			if (parent != nullptr) {
				SPDLOG_TRACE("Node {} is a leaf, propagate labels.", *this);
				parent->label_propagate(controller_actions, environment_actions);
			}
			return;
		}
		// do nothing if the node is already labelled
		if (label != NodeLabel::UNLABELED && !children.empty()) {
			SPDLOG_TRACE("Node is already labelled, abort.");
			return;
		}
		assert(!children.empty());
		// find good and bad child nodes which are already labelled and determine their order (with
		// respect to time). Also keep track of yet unlabelled nodes (both cases, environmental and
		// controller action).
		RegionIndex first_good_controller_step{std::numeric_limits<RegionIndex>::max()};
		RegionIndex first_non_bad_controller_step{std::numeric_limits<RegionIndex>::max()};
		RegionIndex first_non_good_environment_step{std::numeric_limits<RegionIndex>::max()};
		RegionIndex first_bad_environment_step{std::numeric_limits<RegionIndex>::max()};
		for (const auto &child : children) {
			for (const auto &[step, action] : child->incoming_actions) {
				if (child->label == NodeLabel::TOP
				    && controller_actions.find(action) != std::end(controller_actions)) {
					first_good_controller_step = std::min(first_good_controller_step, step);
				} else if (child->label == NodeLabel::BOTTOM
				           && environment_actions.find(action) != std::end(environment_actions)) {
					first_bad_environment_step = std::min(first_bad_environment_step, step);
				} else if (child->label == NodeLabel::UNLABELED
				           && environment_actions.find(action) != std::end(environment_actions)) {
					first_non_good_environment_step = std::min(first_non_good_environment_step, step);
				} else if (child->label == NodeLabel::UNLABELED
				           && controller_actions.find(action) != std::end(controller_actions)) {
					first_non_bad_controller_step = std::min(first_non_bad_controller_step, step);
				}
			}
		}
		SPDLOG_TRACE("First good controller step at {}, first non-good env. action step at {}, first "
		             "bad env. action at {}",
		             first_good_controller_step,
		             first_non_good_environment_step,
		             first_bad_environment_step);
		// cases in which incremental labelling can be applied and recursive calls should be issued
		if (first_good_controller_step < first_non_good_environment_step
		    && first_good_controller_step < first_bad_environment_step) {
			label = NodeLabel::TOP;
			SPDLOG_TRACE("Label with TOP");
			if (parent != nullptr) {
				parent->label_propagate(controller_actions, environment_actions);
			}
		} else if (first_bad_environment_step < first_good_controller_step
		           && first_bad_environment_step < first_non_bad_controller_step) {
			label = NodeLabel::BOTTOM;
			SPDLOG_TRACE("Label with BOTTOM");
			if (parent != nullptr) {
				parent->label_propagate(controller_actions, environment_actions);
			}
		} else if (first_bad_environment_step == std::numeric_limits<RegionIndex>::max()
		           && first_non_good_environment_step == std::numeric_limits<RegionIndex>::max()
		           && first_good_controller_step == std::numeric_limits<RegionIndex>::max()
		           && first_non_bad_controller_step == std::numeric_limits<RegionIndex>::max()) {
			// Here, there is no case where a controller can enforce a good action
			// and no case where the environment can enforce a bad action. Moreover, there is no
			// unlabelled node, as the non-good/non-bad labels also have not been set. Thus, waiting,
			// i.e., no controller action at all solves the case and the node can be labelled as GOOD.
			assert(std::none_of(children.begin(), children.end(), [](const auto &child) {
				return child->label == NodeLabel::UNLABELED;
			}));
			SPDLOG_TRACE(
			  "Label node {} with TOP as all labels are determined and no good controller action is "
			  "available and no bad environment action is possible.",
			  *this);
			label = NodeLabel::TOP;
			if (parent != nullptr) {
				parent->label_propagate(controller_actions, environment_actions);
			}
		}
	}

	/**
	 * @brief Iterator to the beginning of the sequence induced by preorder traversal of the subtree
	 * induced by this node.
	 * @return preorder_iterator<SearchTreeNode<Location, ActionType>>
	 */
	preorder_iterator<SearchTreeNode<Location, ActionType>>
	begin()
	{
		return synchronous_product::begin(this);
	}

	/**
	 * @brief Iterator to the end of the sequence induced by preorder traversal of the subtree induced
	 * by this node.
	 * @return preorder_iterator<SearchTreeNode<Location, ActionType>>
	 */
	preorder_iterator<SearchTreeNode<Location, ActionType>>
	end()
	{
		return synchronous_product::end(this);
	}

	/**
	 * @brief Compares two nodes for equality.
	 * @param other The other node
	 * @return true If both nodes are the same (without considering subtrees)
	 * @return false Otherwise
	 */
	bool
	operator==(const SearchTreeNode<Location, ActionType> &other) const
	{
		return this->words == other.words && this->state == other.state
		       && this->label == other.label
		       //&& this->parent == other.parent && this->children == other.children
		       && this->incoming_actions == other.incoming_actions;
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
	std::set<std::pair<RegionIndex, ActionType>> incoming_actions;
};

} // namespace synchronous_product

template <typename Location, typename ActionType>
void
print_to_ostream(std::ostream &                                                   os,
                 const synchronous_product::SearchTreeNode<Location, ActionType> &node,
                 unsigned int                                                     indent = 0)
{
	using synchronous_product::NodeLabel;
	using synchronous_product::NodeState;
	for (unsigned int i = 0; i < indent; i++) {
		os << "  ";
	}
	os << "(" << indent << ") -> { ";
	// TODO This should be a separate operator
	for (const auto &action : node.incoming_actions) {
		os << "(" << action.first << ", " << action.second << ") ";
	}
	os << "} -> ";
	os << node.words << ": ";
	switch (node.state) {
	case NodeState::UNKNOWN: os << "UNKNOWN"; break;
	case NodeState::GOOD: os << "GOOD"; break;
	case NodeState::BAD: os << "BAD"; break;
	case NodeState::DEAD: os << "DEAD"; break;
	}
	os << " ";
	switch (node.label) {
	case NodeLabel::TOP: os << u8"⊤"; break;
	case NodeLabel::BOTTOM: os << u8"⊥"; break;
	case NodeLabel::UNLABELED: os << u8"?"; break;
	}
	os << '\n';
	for (const auto &child : node.children) {
		print_to_ostream(os, *child, indent + 1);
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
