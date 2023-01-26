/***************************************************************************
 *  types.h - common type definitions
 *
 *  Created:   Thu  9 Sep 09:59:47 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include <fmt/ostream.h>

#include <iostream>
#include <map>
#include <string>

namespace tacos {

using RegionIndex    = unsigned int;
using Time           = double;
using Endpoint       = unsigned int;
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

/** @brief A configuration of a plant, e.g., a TA.
 *
 * A plant configuration consists of a location and a set of clock valuations.
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

template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const PlantConfiguration<LocationT> &configuration)
{
	os << "(" << configuration.location << ", ";
	if (configuration.clock_valuations.empty()) {
		os << "{})";
		return os;
	}
	os << "{ ";
	bool first = true;
	for (const auto &[clock, value] : configuration.clock_valuations) {
		if (first) {
			first = false;
		} else {
			os << ", ";
		}
		os << clock << ": " << value;
	}
	os << " } )";
	return os;
}

} // namespace tacos

namespace fmt {

template <typename LocationT>
struct formatter<tacos::PlantConfiguration<LocationT>> : ostream_formatter
{
};

} // namespace fmt
