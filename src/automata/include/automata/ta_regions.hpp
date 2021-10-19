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

#pragma once

#include "ta_regions.h"

namespace tacos::automata::ta {

template <typename LocationT>
TAConfiguration<LocationT>
get_region_candidate(const RegionalizedConfiguration<LocationT> &regionalized_configuration)
{
	TAConfiguration<LocationT> res;
	res.location = regionalized_configuration.first;
	std::transform(std::begin(regionalized_configuration.second),
	               std::end(regionalized_configuration.second),
	               std::inserter(res.clock_valuations, res.clock_valuations.end()),
	               [&](const std::pair<std::string, RegionIndex> &clock_region) {
		               const std::string &clock_name = clock_region.first;
		               const RegionIndex &region     = clock_region.second;
		               return std::make_pair(clock_name, static_cast<ClockValuation>(region) / 2);
	               });
	return res;
}

template <typename LocationT, typename AP>
RegionIndex
get_maximal_region_index(const TimedAutomaton<LocationT, AP> &ta)
{
	Time largest_constant = ta.get_largest_constant();
	// TODO Note that *all* constants in constraints should be Integer. We should maybe update the
	// type.
	assert(utilities::isInteger<RegionIndex>(largest_constant));
	return RegionIndex(2 * largest_constant + 1);
}

} // namespace tacos::automata::ta
