/***************************************************************************
 *  search_tree.cpp - Search tree data structure for the AB search tree
 *
 *  Created:   Fri 26 Mar 11:33:05 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 ****************************************************************************/

#include "synchronous_product/search_tree.h"

namespace synchronous_product {
std::ostream &
operator<<(std::ostream &os, const synchronous_product::NodeState &node_state)
{
	using synchronous_product::NodeState;
	switch (node_state) {
	case NodeState::UNKNOWN: os << "UNKNOWN"; break;
	case NodeState::GOOD: os << "GOOD"; break;
	case NodeState::BAD: os << "BAD"; break;
	case NodeState::DEAD: os << "DEAD"; break;
	}
	return os;
}

std::ostream &
operator<<(std::ostream &os, const synchronous_product::NodeLabel &node_label)
{
	using synchronous_product::NodeLabel;
	switch (node_label) {
	case NodeLabel::TOP: os << u8"⊤"; break;
	case NodeLabel::BOTTOM: os << u8"⊥"; break;
	case NodeLabel::UNLABELED: os << u8"?"; break;
	case NodeLabel::CANCELED: os << "CANCELED"; break;
	}
	return os;
}

} // namespace synchronous_product
