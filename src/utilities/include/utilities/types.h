/***************************************************************************
 *  types.h - common type definitions
 *
 *  Created:   Thu  9 Sep 09:59:47 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#include <map>
#include <string>

namespace tacos {

using RegionIndex    = unsigned int;
using Time           = double;
using ClockValuation = Time;

/// A clock of a plant.
class Clock
{
public:
	/** Constructor with a specified time.
	 * @param init The initial time
	 */
	constexpr Clock(const Time &init = 0) noexcept : valuation_(init)
	{
	}

	/** Let the clock tick for the given amount of time.
	 * @param diff the amount of time to add to the clock
	 */
	constexpr void
	tick(const Time &diff) noexcept
	{
		valuation_ += diff;
	}

	/** Get the current valuation of the clock
	 * @return The current time of the clock
	 */
	constexpr Time
	get_valuation() const noexcept
	{
		return valuation_;
	}

	/** Reset the clock to 0. */
	constexpr void
	reset() noexcept
	{
		valuation_ = 0;
	}

	/** Implicit conversion to the time value */
	constexpr operator Time() const noexcept
	{
		return get_valuation();
	}

private:
	Time valuation_;
};

using ClockSetValuation = std::map<std::string, Clock>;

/** A Plant Configuration, consisting of a location and a set of clock valuations.
 * @tparam LocationT The location type
 */
template <typename LocationT>
struct PlantConfiguration
{
	/** The current location of the TA */
	LocationT location;
	/** The current clock valuations of the TA */
	ClockSetValuation clock_valuations;

	/** Check if one configuration is lexicographically smaller than the other.
	 * @return true if the first configuration is smaler than the second
	 */
	[[nodiscard]] friend bool
	operator<(const PlantConfiguration<LocationT> &first, const PlantConfiguration<LocationT> &second)
	{
		return std::tie(first.location, first.clock_valuations)
		       < std::tie(second.location, second.clock_valuations);
	}
	/** Check if two configurations are identical.
	 * @return true if both configurations have the same location and clock valuations.
	 */
	[[nodiscard]] friend bool
	operator==(const PlantConfiguration<LocationT> &first,
	           const PlantConfiguration<LocationT> &second)
	{
		return !(first < second) && !(second < first);
	}
};

} // namespace tacos