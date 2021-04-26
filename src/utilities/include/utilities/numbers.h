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

} // namespace utilities
