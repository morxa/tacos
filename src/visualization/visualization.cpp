/***************************************************************************
 *  visualization.cpp - Visualization with graphviz
 *
 *  Created:   Fri 16 Apr 09:16:07 CEST 2021
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

#include "visualization/interactive_tree_to_graphviz.h"

namespace tacos::visualization {

void
print_interactive_help()
{
	fmt::print("\nCommands:\n");
	fmt::print("h:     help\n");
	fmt::print("u:     undo, hide the latest selected node\n");
	fmt::print("i:     switch to insertion mode to select an additional node to display\n");
	fmt::print("n:     switch to navigation mode to move around the search graph\n");
	fmt::print("<num>: select node number <num>\n");
	fmt::print("*:     select all children (only possible in insertion mode)");
	fmt::print("\n");
}

} // namespace tacos::visualization
