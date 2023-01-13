/***************************************************************************
 *  ta_to_graphviz.h - Generate a graphviz representation of a TA
 *
 *  Created:   Fri 16 Apr 09:15:33 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include <automata/ta.h>
#include <fmt/format.h>
#include <utilities/graphviz/graphviz.h>

namespace tacos::visualization {

/** @brief Generate a dot graph from a timed automaton.
 *
 * @param ta The timed automaton to visualize
 * @param show_node_labels If set to false, the nodes are displayed as circles without labels.
 * @return The ta as dot graph
 */
template <typename LocationT, typename ActionT>
utilities::graphviz::Graph
ta_to_graphviz(const automata::ta::TimedAutomaton<LocationT, ActionT> &ta,
               bool                                                    show_node_labels = true)
{
	utilities::graphviz::Graph g;
	if (!show_node_labels) {
		g.set_default_node_property("shape", "point");
	}
	std::map<automata::ta::Location<LocationT>, utilities::graphviz::Node> nodes;
	auto initial_node = g.add_node("");
	initial_node.set_property("shape", "none");
	for (const auto &location : ta.get_locations()) {
		std::stringstream str;
		// Use stream operator of the location.
		str << location;
		nodes.insert(std::make_pair(location, g.add_node(str.str())));
	}
	g.add_edge(initial_node, nodes[ta.get_initial_location()]);
	for (const auto &final_location : ta.get_final_locations()) {
		nodes[final_location].set_property("peripheries", "2");
	}
	for (const auto &[source, transition] : ta.get_transitions()) {
		std::stringstream edge;
		edge << " " << transition.symbol_ << " \n " << transition.clock_constraints_ << " \n "
		     << fmt::format("{{{}}}", fmt::join(transition.clock_resets_, ", ")) << " ";
		g.add_edge(nodes[transition.source_], nodes[transition.target_], edge.str());
	}
	return g;
}

} // namespace tacos::visualization
