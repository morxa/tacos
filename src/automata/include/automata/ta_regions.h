/***************************************************************************
 *  ta_regions.h - Timed Automata regions
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

#ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_REGIONS_H
#define SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_REGIONS_H value

#include "automata.h"
#include "automata/ta.h"
#include "utilities/numbers.h"

#include <iostream>

namespace automata::ta {

using RegionIndex        = size_t;
using RegionSetValuation = std::map<std::string, RegionIndex>;
template <typename LocationT>
using RegionalizedConfiguration = std::pair<LocationT, RegionSetValuation>;
using Integer                   = unsigned; ///< fix integer type

/// A set of one-dimensional regions
struct TimedAutomatonRegions
{
	Integer largestConstant; ///< the largest constant the according clock is compared to

	/// returns the index of the region in which the time-point lies.
	RegionIndex getRegionIndex(ClockValuation timePoint);
};

/// Get a (unregionalized) Configuration of a TimedAutomaton for a given regionalized Configuration
template <typename LocationT>
Configuration<LocationT>
get_region_candidate(const RegionalizedConfiguration<LocationT> &regionalized_configuration);

/**
 * @brief Get the maximal region index from a given timed automaton
 *
 * @tparam LocationT
 * @tparam AP
 * @param ta
 * @return RegionIndex
 */
template <typename LocationT, typename AP>
RegionIndex get_maximal_region_index(const TimedAutomaton<LocationT, AP> &ta);

extern template Configuration<std::string>
get_region_candidate(const RegionalizedConfiguration<std::string> &regionalized_configuration);

extern template RegionIndex
get_maximal_region_index(const TimedAutomaton<std::string, std::string> &ta);
extern template RegionIndex get_maximal_region_index(const TimedAutomaton<std::string, int> &ta);
extern template RegionIndex
get_maximal_region_index(const TimedAutomaton<std::string, unsigned int> &ta);

} // namespace automata::ta

#include "ta_regions.hpp"

#endif /* ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_REGIONS_H */
