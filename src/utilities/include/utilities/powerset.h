/***************************************************************************
 *  powerset.h - power set construction
 *
 *  Created:   Wed 19 Jan 14:43:59 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include <set>

namespace tacos::utilities {

template <typename Element>
std::set<std::set<Element>>
construct_powerset(const std::set<Element> &input)
{
	std::set<std::set<Element>> powerset = {{}};
	for (const auto &symbol : input) {
		auto new_elements = powerset;
		for (const auto &partial_set : powerset) {
			auto new_element = partial_set;
			new_element.insert(symbol);
			new_elements.insert(new_element);
		}
		powerset.merge(new_elements);
	}
	return powerset;
}

} // namespace tacos::utilities
