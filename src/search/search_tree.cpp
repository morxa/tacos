/***************************************************************************
 *  search_tree.cpp - Search tree data structure for the AB search tree
 *
 *  Created:   Fri 26 Mar 11:33:05 CET 2021
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

#include "search/search_tree.h"

namespace tacos::search {
std::ostream &
operator<<(std::ostream &os, const search::NodeState &node_state)
{
	using search::NodeState;
	switch (node_state) {
	case NodeState::UNKNOWN: os << "UNKNOWN"; break;
	case NodeState::GOOD: os << "GOOD"; break;
	case NodeState::BAD: os << "BAD"; break;
	case NodeState::DEAD: os << "DEAD"; break;
	}
	return os;
}

std::ostream &
operator<<(std::ostream &os, const search::NodeLabel &node_label)
{
	using search::NodeLabel;
	switch (node_label) {
	case NodeLabel::TOP: os << u8"⊤"; break;
	case NodeLabel::BOTTOM: os << u8"⊥"; break;
	case NodeLabel::UNLABELED: os << u8"?"; break;
	case NodeLabel::CANCELED: os << "CANCELED"; break;
	}
	return os;
}

} // namespace tacos::search
