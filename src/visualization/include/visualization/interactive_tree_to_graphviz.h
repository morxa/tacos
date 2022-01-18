/***************************************************************************
 *  interactive_tree_to_graphviz.h - Interactively visualize a search graph
 *
 *  Created:   Fri  9 Jul 12:25:26 CEST 2021
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

#include "tree_to_graphviz.h"

#include <fmt/ostream.h>
#include <search/search_tree.h>

namespace tacos::visualization {

/// Print a help message to the logger.
void print_interactive_help();

enum class Mode {
	NAVIGATE,
	INSERT,
	INSERT_AND_FOLLOW,
};

std::ostream &operator<<(std::ostream &os, const Mode &mode);

namespace details {
template <typename ActionT, typename NodeT>
std::map<int, const NodeT *>
create_selector_map(
  const std::map<std::pair<RegionIndex, ActionT>, std::shared_ptr<NodeT>> &children,
  const std::set<NodeT *> &                                                parents = {})
{
	std::map<int, const NodeT *> selector_map;
	int                          node_index = 0;
	for (const auto &node : parents) {
		selector_map[node_index] = node;
		fmt::print("{}: Parent \033[37m{}\033[0m\n", node_index, *node);
		node_index += 1;
	}
	for (const auto &[action, node] : children) {
		selector_map[node_index] = node.get();
		fmt::print("{}: \033[34m({}, {})\033[0m -> \033[37m{}\033[0m\n",
		           node_index,
		           action.first,
		           action.second,
		           *node);
		node_index += 1;
	}
	return selector_map;
}

} // namespace details

/** Interactively visualize a search tree.
 * This allows selecting and unselecting nodes to visualize one-by-one. This is helpful, e.g., to
 * debug a particular controller path.
 * @param search_node The search node to start with (usually the root of the search tree)
 * @param output_path The path of the output file to write after each iteration.
 */
template <typename LocationT, typename ActionT, typename ConstraintSymbolT>
void
search_tree_to_graphviz_interactive(
  const search::SearchTreeNode<LocationT, ActionT, ConstraintSymbolT> *search_node,
  const std::filesystem::path &                                        output_path,
  std::istream &                                                       is = std::cin)
{
	using Node = search::SearchTreeNode<LocationT, ActionT, ConstraintSymbolT>;
	std::vector<const Node *>         selected_nodes = {search_node};
	const Node *                      last_node      = search_node;
	std::function<bool(const Node &)> selector       = [&selected_nodes](const Node &node) {
    return std::find(std::begin(selected_nodes), std::end(selected_nodes), &node)
           != std::end(selected_nodes);
	};
	bool quit = false;
	Mode mode = Mode::INSERT;
	fmt::print("Starting interactive debugger\n");
	print_interactive_help();
	while (!quit) {
		fmt::print("Updating output file {} ...", output_path);
		search_tree_to_graphviz(*search_node, selector).render_to_file(output_path);
		fmt::print(" done!\n");
		fmt::print("{}: Please select a child (or 'h' for help):\n", mode);

		std::map<int, const Node *> selector_map;
		if (mode == Mode::NAVIGATE) {
			selector_map =
			  details::create_selector_map<ActionT, Node>(last_node->get_children(), last_node->parents);
		} else {
			selector_map = details::create_selector_map<ActionT, Node>(last_node->get_children());
		}

		std::string input;
		std::getline(is, input);
		if (input == "q") {
			quit = true;
		} else if (input == "h") {
			print_interactive_help();
		} else if (input == "u") {
			if (selected_nodes.size() <= 1) {
				fmt::print("Cannot remove the last node!\n");
			} else {
				selected_nodes.pop_back();
				last_node = selected_nodes.back();
			}
		} else if (input == "n") {
			mode = Mode::NAVIGATE;
		} else if (input == "i") {
			mode = Mode::INSERT;
		} else if (input == "a") {
			mode = Mode::INSERT_AND_FOLLOW;
		} else if (mode == Mode::INSERT && input == "*") {
			for (const auto &[_, child] : selector_map) {
				selected_nodes.push_back(child);
			}
		} else {
			std::size_t selected = 0;
			try {
				selected = std::stoi(input);
			} catch (std::exception &e) {
				fmt::print("Failed to parse input '{}': {}\n", input, e.what());
				continue;
			}
			if (selected >= selector_map.size()) {
				fmt::print("Invalid input (must be in range [0, {}))\n", last_node->get_children().size());
				continue;
			}
			auto &selected_node = selector_map.at(selected);
			switch (mode) {
			case Mode::NAVIGATE: last_node = selected_node; break;
			case Mode::INSERT: selected_nodes.push_back(selected_node); break;
			case Mode::INSERT_AND_FOLLOW:
				last_node = selected_node;
				selected_nodes.push_back(selected_node);
				break;
			}
		}
	}
}

} // namespace tacos::visualization
