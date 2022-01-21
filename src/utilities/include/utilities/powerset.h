/***************************************************************************
 *  powerset.h - power set construction
 *
 *  Created:   Wed 19 Jan 14:43:59 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include <set>
#include <vector>

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

/** Construct all possible combinations from the input vectors. Each combination consists of exactly
 * one element of each input vector.
 * @param input A vector of vectors of inputs
 * @return A vector of vectors of outputs, where each vector contains one element of each input
 * vector
 */
template <typename Element>
std::vector<std::vector<Element>>
construct_combinations(const std::vector<std::vector<Element>> &input)
{
	std::vector<std::vector<Element>> res = {{}};
	for (const auto &params : input) {
		std::vector<std::vector<Element>> new_res;
		for (const auto &r : res) {
			for (const auto &p : params) {
				std::vector<Element> new_r = r;
				new_r.push_back(p);
				new_res.push_back(new_r);
			}
		}
		res = new_res;
	}
	return res;
}

} // namespace tacos::utilities
