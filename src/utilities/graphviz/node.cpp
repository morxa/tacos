/***************************************************************************
 *  node.cpp - Graphviz node wrapper
 *
 *  Created: Thu 15 Apr 2021 15:49:21 CEST 15:49
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



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
