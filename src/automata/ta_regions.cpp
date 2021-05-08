/***************************************************************************
 *  ta_regions.hpp - Timed Automata regions
 *
 *  Created: Mon Dec 14 16:36:11 CET 2020
 *  Copyright  2020  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
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

#include "automata/ta_regions.h"

#include "automata/automata.h"

namespace automata::ta {

RegionIndex
TimedAutomatonRegions::getRegionIndex(ClockValuation timePoint)
{
	if (timePoint > largestConstant) {
		return std::size_t(2 * largestConstant + 1);
	}
	Integer        intPart = utilities::getIntegerPart<Integer, ClockValuation>(timePoint);
	ClockValuation fraPart = utilities::getFractionalPart<Integer, ClockValuation>(timePoint);
	if (utilities::isNearZero(fraPart)) {
		return std::size_t(2 * intPart);
	} else {
		return std::size_t(2 * intPart + 1);
	}
}

std::vector<ClockConstraint>
get_clock_constraints_from_region_index(ta::RegionIndex     region_index,
                                        ta::RegionIndex     max_region_index,
                                        ConstraintBoundType bound_type)
{
	const bool                   get_lower = bound_type != ConstraintBoundType::UPPER;
	const bool                   get_upper = bound_type != ConstraintBoundType::LOWER;
	std::vector<ClockConstraint> res;
	if (region_index % 2 == 0) {
		if (get_lower && get_upper) {
			return {AtomicClockConstraintT<std::equal_to<Time>>(region_index / 2)};
		}
		if (get_lower && region_index > 0) {
			res.push_back(AtomicClockConstraintT<std::greater_equal<Time>>(region_index / 2));
		}
		if (get_upper) {
			if (region_index == 0) {
				return {AtomicClockConstraintT<std::equal_to<Time>>(0)};
			} else {
				res.push_back(AtomicClockConstraintT<std::less_equal<Time>>(region_index / 2));
			}
		}
		return res;
	} else {
		if (get_lower) {
			res.push_back(AtomicClockConstraintT<std::greater<Time>>(region_index / 2));
		}
		if (get_upper && region_index < max_region_index) {
			res.push_back(AtomicClockConstraintT<std::less<Time>>((region_index + 1) / 2));
		}
		return res;
	}
}

} // namespace automata::ta
