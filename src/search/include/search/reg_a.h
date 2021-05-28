/***************************************************************************
 *  reg_a.h - Definition of the function reg_a(w)
 *
 *  Created:   Fri  5 Feb 14:25:02 CET 2021
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

#include "canonical_word.h"

#include <variant>

namespace search {

/** Compute reg_a(w), which is w with all configuration components from B omitted.
 * The resulting word only contains configurations from the timed automaton A.
 * @param word The word to compute reg_a(word) of
 * @return The word reg_a(word), which is the same as word, but without any configurations from the
 * ATA
 */
template <typename Location, typename ActionType>
CanonicalABWord<Location, ActionType>
reg_a(const CanonicalABWord<Location, ActionType> &word)
{
	CanonicalABWord<Location, ActionType> res;
	for (const auto &partition : word) {
		std::set<ABRegionSymbol<Location, ActionType>> res_i;
		for (const auto &ab_symbol : partition) {
			if (std::holds_alternative<TARegionState<Location>>(ab_symbol)) {
				res_i.insert(ab_symbol);
			}
		}
		// TODO check if we can just skip the whole partition
		if (!res_i.empty()) {
			res.push_back(res_i);
		}
	}
	return res;
}

} // namespace search
