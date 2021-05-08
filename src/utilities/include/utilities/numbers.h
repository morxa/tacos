/***************************************************************************
 *  numbers.h - Utility functions for numbers
 *
 *  Created:   Mon Dec 7 18:18:28 2020 +0100
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

#include "config.h"

#include <cmath>

namespace utilities {

/// checks whether a float is close to zero, based on
/// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
template <typename Float>
bool
isNearZero(Float in, int factor = absolute_epsilon_factor)
{
	return fabs(in) < factor * std::numeric_limits<Float>::epsilon();
}

template <typename Float>
bool
is_approx_same(const Float &first, const Float &second, int factor = absolute_epsilon_factor)
{
	return isNearZero(first - second, factor);
}

template <typename Integer, typename Float>
Integer
getIntegerPart(Float in)
{
	return Integer(std::floor(in));
}

template <typename Integer, typename Float>
Float
getFractionalPart(Float in)
{
	return (in - Float(getIntegerPart<Integer, Float>(in)));
}

template <typename Integer, typename Float>
bool
isInteger(Float in)
{
	return isNearZero(getFractionalPart<Integer, Float>(in));
}

/// Sort into partitions by the fractional parts.
template <typename Float>
struct ApproxFloatComparator
{
	/** @brief Compare two floats with approximate comparison.
	 * @param v1 The first float
	 * @param v2 The second float
	 * @return true If v1 and v2 and not approximately the same and if v1 is smaller than v2.
	 */
	bool
	operator()(const Float &v1, const Float &v2) const
	{
		if (is_approx_same(v1, v2)) {
			return false;
		} else {
			return v1 < v2;
		}
	}
};

} // namespace utilities
