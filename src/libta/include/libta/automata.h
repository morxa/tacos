/***************************************************************************
 *  automata.h - Generic automata definitions
 *
 *  Created: Thu 28 May 2020 14:09:49 CEST 14:09
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#pragma once

#include <functional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace automata {

using Location       = std::string;
using Symbol         = std::string;
using Time           = double;
using ClockValuation = Time;
using Endpoint       = unsigned int;
using TimedWord      = std::vector<std::pair<Symbol, Time>>;

/// A clock of a timed automaton.
class Clock
{
public:
	/** Constructor. */
	constexpr Clock() noexcept : valuation_{0}
	{
	}

	/** Let the clock tick for the given amount of time.
	 * @param diff the amount of time to add to the clock
	 */
	constexpr void
	tick(const Time &diff) noexcept
	{
		valuation_ += diff;
	};

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

private:
	Time valuation_;
};

/// Invalid location encountered
/*** This exception is thrown when some location (e.g., as part of a transition) is not  part of a
 * timed automaton.
 */
class InvalidLocationException : public std::invalid_argument
{
public:
	/** Constructor
	 * @param location The name of the invalid location
	 */
	explicit InvalidLocationException(const Location &location)
	: std::invalid_argument("Invalid location: " + location)
	{
	}
};

/// Invalid clock encountered
/*** This exception is thrown when some clock (e.g., as part of a transition) is not  part of a
 * timed automaton.
 */
class InvalidClockException : public std::invalid_argument
{
public:
	/** Constructor
	 * @param clock_name The name of the invalid clock
	 */
	explicit InvalidClockException(const std::string &clock_name)
	: std::invalid_argument("Invalid clock: " + clock_name)
	{
	}
};

/// An atomic clock constraint.
/**
 * This is a templated atomic constraint, where the template parameter is the comparison operator,
 * e.g., to define a constraint <tt>x <= 3</tt> on a clock x, use
 * <tt>AtomicClockConstraintT<std::less_equal>(3)</tt>.
 * @tparam Comp the comparison operator, e.g., std::less
 */
template <class Comp>
class AtomicClockConstraintT
{
public:
	/** Constructor.
	 * @param comparand the constant to compare a clock value against
	 */
	AtomicClockConstraintT(const Endpoint &comparand) : comparand_(comparand)
	{
	}
	/** Check if the clock constraint is satisfied.
	 * @param valuation the valuation of a clock
	 * @return true if the constraint is satisfied
	 */
	constexpr bool
	is_satisfied(const Time &valuation) const
	{
		return Comp()(valuation, comparand_);
	}

private:
	const Endpoint comparand_;
};

using ClockConstraint = std::variant<AtomicClockConstraintT<std::less<Time>>,
                                     AtomicClockConstraintT<std::less_equal<Time>>,
                                     AtomicClockConstraintT<std::equal_to<Time>>,
                                     AtomicClockConstraintT<std::not_equal_to<Time>>,
                                     AtomicClockConstraintT<std::greater_equal<Time>>,
                                     AtomicClockConstraintT<std::greater<Time>>>;

bool is_satisfied(const ClockConstraint &constraint, const ClockValuation &valuation);

} // namespace automata
