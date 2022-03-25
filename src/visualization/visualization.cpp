/***************************************************************************
 *  visualization.cpp - Visualization with graphviz
 *
 *  Created:   Fri 16 Apr 09:16:07 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include "visualization/interactive_tree_to_graphviz.h"

namespace tacos::visualization {

void
print_interactive_help()
{
	fmt::print("\nCommands:\n");
	fmt::print("h:     help\n");
	fmt::print("u:     undo, hide the latest selected node\n");
	fmt::print("i:     switch to insertion mode to select an additional node to display\n");
	fmt::print(
	  "a:     switch to insertion+follow mode to select and follow an additional node to display\n");
	fmt::print("n:     switch to navigation mode to move around the search graph\n");
	fmt::print("<num>: select node number <num>\n");
	fmt::print("*:     select all children (only possible in insertion mode)");
	fmt::print("\n");
}

std::ostream &
operator<<(std::ostream &os, const Mode &mode)
{
	switch (mode) {
	case Mode::NAVIGATE: os << "NAVIGATE"; break;
	case Mode::INSERT: os << "INSERT"; break;
	case Mode::INSERT_AND_FOLLOW: os << "INSERT AND FOLLOW"; break;
	}
	return os;
}

} // namespace tacos::visualization
