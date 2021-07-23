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

#include <search/search_tree.h>

namespace visualization {

/** Interactively visualize a search tree.
 * This allows selecting and unselecting nodes to visualize one-by-one. This is helpful, e.g., to
 * debug a particular controller path.
 * @param search_node The search node to start with (usually the root of the search tree)
 * @param output_path The path of the output file to write after each iteration.
 */
template <typename LocationT, typename ActionT>
void
search_tree_to_graphviz_interactive(const search::SearchTreeNode<LocationT, ActionT> *search_node,
                                    const std::filesystem::path &                     output_path)
{
	using Node                                       = search::SearchTreeNode<LocationT, ActionT>;
	std::vector<const Node *>         selected_nodes = {search_node};
	const Node *                      last_node      = search_node;
	std::function<bool(const Node &)> selector       = [&selected_nodes](const Node &node) {
    return std::find(std::begin(selected_nodes), std::end(selected_nodes), &node)
           != std::end(selected_nodes);
	};
	bool quit = false;
	while (!quit) {
		fmt::print("Updating output file {} ...", output_path);
		search_tree_to_graphviz(*search_node, selector).render_to_file(output_path);
		fmt::print(" done!\n");
		fmt::print("Please select a child to expand (or 'q' for quit):\n");
		last_node->get_children();
		std::map<int, const Node *> selector_map;
		{
			int node_index = 0;
			for (const auto &[action, node] : last_node->get_children()) {
				selector_map[node_index] = node.get();
				fmt::print("{}: \033[34m({}, {})\033[0m -> \033[37m{}\033[0m\n",
				           node_index++,
				           action.first,
				           action.second,
				           *node);
			}
		}

		std::string input;
		std::getline(std::cin, input);
		if (input == "q") {
			quit = true;
		} else if (input == "u") {
			if (selected_nodes.size() <= 1) {
				fmt::print("Cannot remove the last node!\n");
			} else {
				selected_nodes.pop_back();
				last_node = selected_nodes.back();
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
			selected_nodes.push_back(selected_node);
			last_node = selected_node;
		}
	}
}

} // namespace visualization
