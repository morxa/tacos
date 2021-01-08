#pragma once
#include "automata.h"
#include "utilities/numbers.h"

#include <iostream>

namespace automata {
namespace ta {

/// A set of one-dimensional regions
struct TimedAutomatonRegions
{
	using Integer = unsigned; ///< fix integer type
	Integer largestConstant;  ///< the largest constant the according clock is compared to

	/// returns the index of the region in which the time-point lies.
	std::size_t
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

} // namespace ta
} // namespace automata
