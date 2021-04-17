/***************************************************************************
 *  ta_to_graphviz.h - Generate a graphviz representation of a TA
 *
 *  Created:   Fri 16 Apr 09:15:33 CEST 2021
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

#include <automata/ta.h>
#include <fmt/format.h>
#include <utilities/graphviz/graphviz.h>

/** Generate a dot graph from a timed automaton.
 * @param ta The timed automaton to visualize
 * @return The ta as dot graph
 */
template <typename LocationT, typename ActionT>
utilities::graphviz::Graph
ta_to_graphviz(const automata::ta::TimedAutomaton<LocationT, ActionT> &ta)
{
	utilities::graphviz::Graph                                             g;
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
		edge << " " << transition.symbol_ << "\n"
		     << transition.clock_constraints_ << "\n"
		     << fmt::format("{{{}}}", fmt::join(transition.clock_resets_, ", "));
		g.add_edge(nodes[transition.source_], nodes[transition.target_], edge.str());
	}
	return g;
}
