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

} // namespace automata::ta
