/***************************************************************************
 *  search_tree.h - Search tree data structure for the AB search tree
 *
 *  Created:   Mon  1 Feb 15:58:24 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include "automata/ta_regions.h"
#include "canonical_word.h"
#include "reg_a.h"

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

namespace tacos::search {

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
	CANCELED,
};

/** The reason for the current label, used for more detailed output */
enum class LabelReason {
	UNKNOWN,
	GOOD_NODE,
	BAD_NODE,
	DEAD_NODE,
	NO_ATA_SUCCESSOR,
	MONOTONIC_DOMINATION,
	NO_BAD_ENV_ACTION,
	GOOD_CONTROLLER_ACTION_FIRST,
	BAD_ENV_ACTION_FIRST,
	ALL_CONTROLLER_ACTIONS_BAD,
};

/** @brief A node in the search tree
 * @see TreeSearch */
template <typename Location, typename ActionType, typename ConstraintSymbolType = ActionType>
class SearchTreeNode
{
public:
	/** Construct a node.
	 * @param words The CanonicalABWords of the node (being of the same reg_a class)
	 */
	SearchTreeNode(const std::set<CanonicalABWord<Location, ConstraintSymbolType>> &words)
	: words(words)
	{
		// The constraints must be either over locations or over actions.
		static_assert(std::is_same_v<Location, ConstraintSymbolType>
		              || std::is_same_v<ActionType, ConstraintSymbolType>);
		// All words must have the same reg_a.
		assert(std::all_of(std::begin(words), std::end(words), [&words](const auto &word) {
			return words.empty() || reg_a(*std::begin(words)) == reg_a(word);
		}));
	}

	/** @brief Set the node label and optionally cancel the children.
	 * @param new_label The new node label
	 * @param cancel_children If true, cancel children after setting the node label
	 */
	void
	set_label(NodeLabel new_label, bool cancel_children = false)
	{
		assert(new_label != NodeLabel::UNLABELED);
		// Check if the node has been labeled before. This is an error, unless either the old or the new
		// label is CANCELED. This is okay, as we may try to cancel a node that has been labeled in the
		// meantime (or vice versa).
		// TODO load label only once and store locally
		if (label != NodeLabel::UNLABELED && label != NodeLabel::CANCELED
		    && new_label != NodeLabel::CANCELED && label != new_label) {
			throw std::logic_error(fmt::format(
			  "Trying to set node label to {}, but it is already set to {}", new_label, label.load()));
		}
		if (label == NodeLabel::UNLABELED) {
			SPDLOG_DEBUG(
			  "Labeling {} {} with {}, reason: {}", fmt::ptr(this), *this, new_label, label_reason);
			label = new_label;
			if (cancel_children) {
				for (const auto &action_child : children) {
					auto child = std::get<1>(action_child);
					if (std::all_of(std::begin(child->parents),
					                std::end(child->parents),
					                [&child](const auto &parent) {
						                return parent == child.get() || parent->label != NodeLabel::UNLABELED;
					                })) {
						child->set_label(NodeLabel::CANCELED, true);
					}
				}
			}
		}
	}

	/** Reset the label of the canceled node.
	 * This expects the node to have the label canceled. If this is not the case, the label is not
	 * changed.
	 */
	void
	reset_label()
	{
		NodeLabel expected_label = NodeLabel::CANCELED;
		label.compare_exchange_strong(expected_label, NodeLabel::UNLABELED);
	}

	/**
	 * @brief Implements incremental labeling during search, bottom up. Nodes are labelled as soon
	 * as their label state can definitely be determined either because they are leaf-nodes or
	 * because the labeling of child nodes permits to determine a label (see details).
	 * @details Leaf-nodes can directly be labelled (in most cases?), the corresponding label pushed
	 * upwards in the search tree may allow for shortening the search significantly in the following
	 * cases: 1) A child is labelled "BAD" and there is no control-action which can be taken before
	 * that is labelled "GOOD" -> the node can be labelled as "BAD". 2) A child is labelled "GOOD"
	 * and came from a control-action and there is no non-"GOOD" environmental-action happening
	 * before -> the node can be labelled "GOOD". The call should be propagated to the parent node
	 * in case the labelling has been determined.
	 * @param controller_actions The set of controller actions
	 * @param environment_actions The set of environment actions
	 * @param cancel_children If true, cancel children if a node is labeled
	 */
	void
	label_propagate(const std::set<ActionType> &controller_actions,
	                const std::set<ActionType> &environment_actions,
	                bool                        cancel_children = false)
	{
		// SPDLOG_TRACE("Call propagate on node {}", *this);
		if (is_expanding) {
			SPDLOG_DEBUG("Cancelling node propagation on {}, currently expanding", *this);
			return;
		}
		if (!is_expanded) {
			SPDLOG_DEBUG("Cancelling node propagation on {}, node is not expanded yet", *this);
			return;
		}
		// leaf-nodes should always be labelled directly
		assert(!children.empty() || label != NodeLabel::UNLABELED);
		// if not already happened: call recursively on parent node
		if (children.empty()) {
			assert(label != NodeLabel::UNLABELED);
			SPDLOG_TRACE("Node is a leaf, propagate labels.", *this);
			for (const auto &parent : parents) {
				if (parent != this) {
					parent->label_propagate(controller_actions, environment_actions, cancel_children);
				}
			}
			return;
		}
		// do nothing if the node is already labelled
		if (label != NodeLabel::UNLABELED) {
			SPDLOG_TRACE("Node is already labelled, abort.");
			return;
		}
		assert(!children.empty());

		// Find good and bad child nodes which are already labelled and determine their order (with
		// respect to time). Also keep track of yet unlabelled nodes (both cases, environmental and
		// controller action).
		constexpr auto max = std::numeric_limits<RegionIndex>::max();
		RegionIndex    first_good_controller_step{max};
		RegionIndex    first_non_bad_controller_step{max};
		RegionIndex    first_non_good_environment_step{max};
		RegionIndex    first_bad_environment_step{max};
		bool           has_enviroment_step{false};
		for (const auto &[timed_action, child] : children) {
			// Copy label to avoid races while checking the conditions below.
			const NodeLabel child_label = child->label;
			const auto &[step, action]  = timed_action;
			if (controller_actions.find(action) != std::end(controller_actions)) {
				if (child_label == NodeLabel::TOP || child.get() == this) {
					first_good_controller_step = std::min(first_good_controller_step, step);
				} else if (child_label == NodeLabel::UNLABELED) {
					first_non_bad_controller_step = std::min(first_non_bad_controller_step, step);
				}
			} else if (environment_actions.find(action) != std::end(environment_actions)) {
				has_enviroment_step = true;
				if (child_label == NodeLabel::BOTTOM) {
					first_bad_environment_step = std::min(first_bad_environment_step, step);
				} else if (child.get() != this && child_label == NodeLabel::UNLABELED) {
					first_non_good_environment_step = std::min(first_non_good_environment_step, step);
				}
			}
		}
		SPDLOG_TRACE("First good ctl step at {}, "
		             "first non-bad ctl step at {}, "
		             "first non-good env step at {}, "
		             "first bad env step at {}",
		             first_good_controller_step,
		             first_non_bad_controller_step,
		             first_non_good_environment_step,
		             first_bad_environment_step);

		if (first_good_controller_step
		    < std::min(first_bad_environment_step, first_non_good_environment_step)) {
			// The controller can just select the good controller action.
			label_reason = LabelReason::GOOD_CONTROLLER_ACTION_FIRST;
			set_label(NodeLabel::TOP, cancel_children);
		} else if (has_enviroment_step
		           && std::min(first_bad_environment_step, first_non_good_environment_step)
		                == std::numeric_limits<RegionIndex>::max()) {
			// There is an environment action and no environment action is bad
			// -> the controller can just select all environment actions
			label_reason = LabelReason::NO_BAD_ENV_ACTION;
			set_label(NodeLabel::TOP, cancel_children);
		} else if (!has_enviroment_step && first_good_controller_step == max
		           && first_non_bad_controller_step == max) {
			// All controller actions must be bad (otherwise we would be in the first case)
			// -> no controller strategy
			label_reason = LabelReason::ALL_CONTROLLER_ACTIONS_BAD;
			set_label(NodeLabel::BOTTOM, cancel_children);
		} else if (has_enviroment_step && first_bad_environment_step < max
		           && first_bad_environment_step
		                <= std::min(first_good_controller_step, first_non_bad_controller_step)) {
			// There must be an environment action (otherwise case 3) and one of them must be bad
			// (otherwise case 2).
			assert(first_bad_environment_step < std::numeric_limits<RegionIndex>::max());
			label_reason = LabelReason::BAD_ENV_ACTION_FIRST;
			set_label(NodeLabel::BOTTOM, cancel_children);
		}
		if (label != NodeLabel::UNLABELED) {
			for (const auto &parent : parents) {
				if (parent != this) {
					parent->label_propagate(controller_actions, environment_actions, cancel_children);
				}
			}
		}
	}

	/**
	 * @brief Compares two nodes for equality.
	 * @param other The other node
	 * @return true If both nodes are the same (without considering subtrees)
	 * @return false Otherwise
	 */
	bool
	operator==(const SearchTreeNode<Location, ActionType, ConstraintSymbolType> &other) const
	{
		return this->words == other.words && this->state == other.state && this->label == other.label
		  //&& this->parent == other.parent && this->children == other.children
		  ;
	}

	/** Get a reference to the map of children.
	 * @return The children
	 */
	const auto &
	get_children() const
	{
		return children;
	}

	/** Add a child to the node.
	 * @param action Taking this action in the current node leads to the new child node
	 * @param node The new child
	 */
	void
	add_child(const std::pair<RegionIndex, ActionType> &action, std::shared_ptr<SearchTreeNode> node)
	{
		if (!children.insert(std::make_pair(action, node)).second) {
			throw std::invalid_argument(fmt::format("\n{}\nCannot add child node \n{}\n, node already "
			                                        "has child \n{}\n with the same action ({}, {})",
			                                        *this,
			                                        *node,
			                                        *children.at(action),
			                                        action.first,
			                                        action.second));
		}
		node->min_total_region_increments =
		  std::min(node->min_total_region_increments, min_total_region_increments + action.first);
		node->parents.insert(this);
	}

	/** The words of the node */
	std::set<CanonicalABWord<Location, ConstraintSymbolType>> words;
	/** The state of the node */
	std::atomic<NodeState> state = NodeState::UNKNOWN;
	/** Whether we have a successful strategy in the node */
	std::atomic<NodeLabel> label = NodeLabel::UNLABELED;
	/** The parent of the node, this node was directly reached from the parent */
	std::set<SearchTreeNode *> parents = {};
	/** Whether the node has been expanded. This is used for multithreading, in particular to check
	 * whether we can access the children already. */
	std::atomic_bool is_expanded{false};
	/** Whether the node is currently being expanded. */
	std::atomic_bool is_expanding{false};
	/** A more detailed description for the node that explains the current label. */
	LabelReason label_reason = LabelReason::UNKNOWN;
	/** The current regionalized minimal total time to reach this node */
	RegionIndex min_total_region_increments = std::numeric_limits<RegionIndex>::max();

private:
	/** A list of the children of the node, which are reachable by a single transition */
	// TODO change container with custom comparator to set to avoid duplicates (also better
	// performance)
	std::map<std::pair<RegionIndex, ActionType>, std::shared_ptr<SearchTreeNode>> children = {};
};

/** Print a node state. */
std::ostream &operator<<(std::ostream &os, const search::NodeState &node_state);
/** Print a node label. */
std::ostream &operator<<(std::ostream &os, const search::NodeLabel &node_label);
/** Print a label reason. */
std::ostream &operator<<(std::ostream &os, const search::LabelReason &reason);

/** @brief Print a SearchTreeNode, optionally the whole tree.
 * By default, just print information about the node itself on a single line. Optionally, also print
 * all its children, effectively printing the whole sub-tree.
 * @param os The stream to print to
 * @param node The node to print
 * @param print_children If true, also print the node's children (not implemented)
 * @param indent The indentation to insert before this node, should be the distance to the root node
 */
template <typename Location, typename ActionType, typename ConstraintSymbolType>
void
print_to_ostream(std::ostream                                                             &os,
                 const search::SearchTreeNode<Location, ActionType, ConstraintSymbolType> &node,
                 __attribute__((unused)) bool         print_children = false,
                 __attribute__((unused)) unsigned int indent         = 0)
{
	os << node.words << ": " << node.state << " " << node.label;
}

/** Print a node
 * @param os The ostream to print to
 * @param node The node to print
 * @return A reference to the ostream
 */
template <typename Location, typename ActionType, typename ConstraintSymbolType>
std::ostream &
operator<<(std::ostream                                                             &os,
           const search::SearchTreeNode<Location, ActionType, ConstraintSymbolType> &node)
{
	print_to_ostream(os, node);
	return os;
}

/** Print a node to a string.
 * @param node The node to print
 * @param print_children If true, also print the node's children
 * @return A string representation of the node
 */
template <typename Location, typename ActionType, typename ConstraintSymbolType>
std::string
node_to_string(const search::SearchTreeNode<Location, ActionType, ConstraintSymbolType> &node,
               bool print_children = false)
{
	std::stringstream str;
	print_to_ostream(str, node, print_children);
	return str.str();
}

/** Print a vector of node pointers
 * @param os The ostream to print to
 * @param nodes The node pointers to print
 * @return A reference to the ostream
 */
template <typename Location, typename ActionType, typename ConstraintSymbolType>
std::ostream &
operator<<(
  std::ostream &os,
  const std::vector<
    std::shared_ptr<search::SearchTreeNode<Location, ActionType, ConstraintSymbolType>>> &nodes)
{
	for (const auto &node : nodes) {
		os << *node;
	}
	return os;
}

} // namespace tacos::search

namespace fmt {

template <>
struct formatter<tacos::search::NodeState> : ostream_formatter
{
};

template <>
struct formatter<tacos::search::NodeLabel> : ostream_formatter
{
};

template <>
struct formatter<tacos::search::LabelReason> : ostream_formatter
{
};

template <typename Location, typename ActionType, typename ConstraintSymbolType>
struct formatter<tacos::search::SearchTreeNode<Location, ActionType, ConstraintSymbolType>>
: ostream_formatter
{
};

template <typename Location, typename ActionType, typename ConstraintSymbolType>
struct formatter<std::vector<
  std::shared_ptr<tacos::search::SearchTreeNode<Location, ActionType, ConstraintSymbolType>>>>
: ostream_formatter
{
};

} // namespace fmt
