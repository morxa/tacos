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
