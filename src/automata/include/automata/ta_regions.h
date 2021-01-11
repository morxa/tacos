#pragma once
#include "automata.h"
#include "automata/ta.h"
#include "utilities/numbers.h"

#include <iostream>

namespace automata {
namespace ta {

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
	RegionIndex
	getRegionIndex(ClockValuation timePoint)
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
};

/// Get a (unregionalized) Configuration of a TimedAutomaton for a given regionalized Configuration
template <typename LocationT>
Configuration<LocationT>
get_region_candidate(const RegionalizedConfiguration<LocationT> &regionalized_configuration)
{
	Configuration<LocationT> res;
	res.first = regionalized_configuration.first;
	std::transform(std::begin(regionalized_configuration.second),
	               std::end(regionalized_configuration.second),
	               std::inserter(res.second, res.second.end()),
	               [&](const std::pair<std::string, RegionIndex> &clock_region) {
		               const std::string &clock_name = clock_region.first;
		               const RegionIndex &region     = clock_region.second;
		               return std::make_pair(clock_name, static_cast<ClockValuation>(region) / 2);
	               });
	return res;
}

} // namespace ta
} // namespace automata
