/***************************************************************************
 *  powerset.h - power set construction
 *
 *  Created:   Wed 19 Jan 14:43:59 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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
