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
	bool get_lower = false;
	bool get_upper = false;
	switch (bound_type) {
	case ConstraintBoundType::BOTH:
		get_lower = true;
		get_upper = true;
		break;
	case ConstraintBoundType::LOWER: get_lower = true; break;
	case ConstraintBoundType::UPPER: get_upper = true; break;
	}
	std::vector<ClockConstraint> res;
	if (get_upper && region_index < max_region_index) {
		if (region_index % 2 == 0) {
			res.push_back(AtomicClockConstraintT<std::less_equal<Time>>(region_index / 2));
		} else {
			res.push_back(AtomicClockConstraintT<std::less<Time>>((region_index + 1) / 2));
		}
	}
	if (get_lower && region_index > 0) {
		if (region_index % 2 == 0) {
			res.push_back(AtomicClockConstraintT<std::greater_equal<Time>>(region_index / 2));
		} else {
			res.push_back(AtomicClockConstraintT<std::greater<Time>>(region_index / 2));
		}
	}
	return res;
}

} // namespace automata::ta
