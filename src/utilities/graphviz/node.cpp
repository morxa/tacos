/***************************************************************************
 *  node.cpp - Graphviz node wrapper
 *
 *  Created: Thu 15 Apr 2021 15:49:21 CEST 15:49
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
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "utilities/graphviz/graphviz.h"

namespace tacos::utilities::graphviz {

Node::Node(Agnode_t *ag_node) : node(ag_node)
{
}

void
Node::set_property(const std::string &property, const std::string &value)
{
	if (node) {
		std::string p = property;
		std::string v = value;
		agsafeset(node, p.data(), v.data(), std::string().data());
	}
}

} // namespace tacos::utilities::graphviz
