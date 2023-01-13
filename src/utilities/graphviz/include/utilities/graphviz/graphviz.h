/***************************************************************************
 *  graphviz.h - A C++ thin wrapper for cgraph
 *
 *  Created:   Wed 14 Apr 22:32:42 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>

#include <filesystem>
#include <optional>
#include <string>

// cgraph defines those, do not leak the definitions.
#undef TRUE
#undef FALSE

/// A thin C++ wrapper around the cgraph library.
namespace tacos::utilities::graphviz {

/// The edge of the graph (e.g., undirected or directed).
enum class GraphType {
	Undirected,       ///< An undirected graph.
	StrictUndirected, ///< An undirected graph without multi-edges.
	Directed,         ///< A directed graph.
	StrictDirected,   ///< A directed graph without multi-edges.
};

class Graph;

/** @brief A wrapper for a node in graphviz. */
class Node
{
	friend Graph;

public:
	/** Default constructor. */
	Node() : node(nullptr)
	{
	}
	/** Set a property to the given value.
	 * @param property The property to set, e.g., label
	 * @param value The value to set, e.g., the value of the label
	 */
	void set_property(const std::string &property, const std::string &value);
	/** Get the underlying node.
	 * @return A pointer to the cgraph node.
	 */
	Agnode_t *
	get_agnode() const
	{
		return node;
	}

private:
	Node(Agnode_t *node);

private:
	Agnode_t *node;
};

/** @brief A simple wrapper to create graphs with graphviz. */
class Graph
{
public:
	/** Create a non-strict directed graph. */
	Graph();

	/** Create a graph with a custom name and type.
	 * @param name The name of the graph.
	 * @param type the type of the graph.
	 */
	Graph(const std::string &name, GraphType type);

	/** @brief Destructor.
	 * Frees all resources that were allocated while creating the graph. */
	~Graph();

	/** Set a default node property for all nodes of the graph.
	 * @param property The default property to set, e.g., shape
	 * @param value The value of the default property
	 */
	void set_default_node_property(const std::string &property, const std::string &value);

	/** Add a node to the graph.
	 * @param label The node label
	 * @param identifier An optional identifier. By default, a unique identifier is automatically
	 * generated during construction.
	 */
	Node add_node(std::string label, std::optional<std::string> identifier = std::nullopt);

	/** Add an edge to the graph.
	 * If the source or target node does not exist, it will be created.
	 * @param source The name of the source node of the edge
	 * @param target The nam eof the target node of the edge
	 * @param label The edge label
	 */
	void add_edge(const Node &source, const Node &target, std::string label = "");

	/** Create a dot representation of the graph.
	 * @return The dot representation as string
	 */
	std::string to_dot();

	/** Set a graph property.
	 * @param property The property to set, e.g., rankdir
	 * @param value The value of the property
	 */
	void set_property(const std::string &property, const std::string &value);

	/** Render the graph to a file.
	 * @param output_path The path to the output file, the suffix must match a suitable graphviz
	 * engine.
	 */
	void render_to_file(const std::filesystem::path &output_path);

	/** Check if the graph has a node with the given identifier.
	 * @param identifier The identifier of the node to check for
	 * @return true if the node exists
	 */
	bool has_node(std::string identifier);

	/** Get an existing node from the graph.
	 * @param identifier The identifier of the node to get
	 * @return The node if it exists
	 */
	std::optional<Node> get_node(std::string identifier);

private:
	void        layout();
	std::string graph_name;
	Agraph_t   *graph{nullptr};
	GVC_t      *context{nullptr};
	bool        layout_created{false};
	std::size_t last_node_id{0};
	std::size_t last_edge_id{0};
};

} // namespace tacos::utilities::graphviz
