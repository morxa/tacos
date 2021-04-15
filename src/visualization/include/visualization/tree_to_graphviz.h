/***************************************************************************
 *  tree_to_graphviz.h - Convert a search tree to a Graphviz graph
 *
 *  Created:   Thu 15 Apr 16:59:15 CEST 2021
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

#include <synchronous_product/search_tree.h>
#include <utilities/graphviz/graphviz.h>

#include <sstream>
#include <vector>

namespace visualization {

template <typename LocationT, typename ActionT>
utilities::graphviz::Node
add_search_node_to_graph(const synchronous_product::SearchTreeNode<LocationT, ActionT> *search_node,
                         utilities::graphviz::Graph *                                   graph)
{
	std::vector<std::string> words_labels;
	for (const auto &word : search_node->words) {
		std::vector<std::string> word_labels;
		for (const auto &word_partition : word) {
			std::stringstream str;
			str << word_partition;
			word_labels.push_back(str.str());
		}
		words_labels.push_back(fmt::format("{{ {} }}", fmt::join(word_labels, "|")));
	}
	std::vector<std::string> incoming_action_labels;
	for (const auto &incoming_action : search_node->incoming_actions) {
		incoming_action_labels.push_back(
		  fmt::format("({}, {})", incoming_action.first, incoming_action.second));
	}

	utilities::graphviz::Node node{graph->add_node(fmt::format("{{{}}}|{}",
	                                                           fmt::join(incoming_action_labels, "|"),
	                                                           fmt::join(words_labels, "|")))};
	// node.set_property("color", "red");
	if (search_node->label == synchronous_product::NodeLabel::TOP) {
		node.set_property("color", "green");
	} else if (search_node->label == synchronous_product::NodeLabel::BOTTOM) {
		node.set_property("color", "red");
	}
	for (const auto &child : search_node->children) {
		auto child_node = add_search_node_to_graph(child.get(), graph);
		graph->add_edge(node, child_node, "");
	}
	return node;
}

template <typename LocationT, typename ActionT>
utilities::graphviz::Graph
search_tree_to_graphviz(const synchronous_product::SearchTreeNode<LocationT, ActionT> &search_node)
{
	utilities::graphviz::Graph graph;
	add_search_node_to_graph(&search_node, &graph);
	return graph;
}

} // namespace visualization
