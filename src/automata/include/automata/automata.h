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

#include <boost/format.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace automata {

class Clock;

using Symbol            = std::string;
using Time              = double;
using ClockValuation    = Time;
using ClockSetValuation = std::map<std::string, Clock>;
using Endpoint          = unsigned int;
using TimedWord         = std::vector<std::pair<Symbol, Time>>;

/// Invalid timed word, e.g., first time is not initialized at 0.
class InvalidTimedWordException : public std::invalid_argument
{
public:
	/** Constructor.
	 * @param msg The exact error
	 */
	InvalidTimedWordException(const std::string &msg) : std::invalid_argument(msg)
	{
	}
};

/// A clock of a timed automaton.
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
	operator Time() const noexcept
	{
		return get_valuation();
	}

private:
	Time valuation_;
};

/// Convert a set of clocks into a set of clock valuations.
ClockSetValuation get_valuations(const std::map<std::string, Clock> &clocks);

/// Create a map of clocks from a ClockSetValuation.
std::map<std::string, Clock> create_clocks(const ClockSetValuation &clock_valuations);

/// Invalid location encountered
/*** This exception is thrown when some location (e.g., as part of a transition) is not  part of a
 * timed automaton.
 */
template <typename Location>
class InvalidLocationException : public std::invalid_argument
{
public:
	/** Constructor
	 * @param location The name of the invalid location
	 */
	explicit InvalidLocationException(const Location &location)
	: std::invalid_argument(str(boost::format("Invalid location: %1%") % location))
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
	: std::invalid_argument(str(boost::format("Invalid clock: %1%") % clock_name))
	{
	}
};

/// Invalid clock constraint comparison operator
/*** This exception is thrown if a clock constraint uses an unexpected comparison operator.
 */
class InvalidClockComparisonOperatorException : public std::invalid_argument
{
public:
	/** Constructor */
	explicit InvalidClockComparisonOperatorException()
	: std::invalid_argument("Invalid clock comparison operator")
	{
	}
};

/// Invalid symbol encountered
/** This exception is thrown if a TimedAutomaton encounters a symbol that is not in its alphabet.*/
class InvalidSymbolException : public std::invalid_argument
{
public:
	/** Constructor */
	explicit InvalidSymbolException(const Symbol &symbol)
	: std::invalid_argument("Invalid symbol '" + symbol + "'")
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
	/** Compare two clock constraints. */
	friend bool
	operator==(const AtomicClockConstraintT<Comp> &lhs, const AtomicClockConstraintT<Comp> &rhs)
	{
		return lhs.comparand_ == rhs.comparand_;
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

	/**
	 * @brief Get the comparand
	 * @return const Time&
	 */
	Endpoint
	get_comparand() const
	{
		return comparand_;
	}

	/** Print an AtomicClockConstraintT to an ostream.
	 * @param os The ostream to print to
	 * @param constraint The constraint to print
	 * @return A reference to the ostream
	 */
	friend std::ostream &
	operator<<(std::ostream &os, const AtomicClockConstraintT &constraint)
	{
		os << "x ";
		if constexpr (std::is_same_v<Comp, std::less<Time>>) {
			os << "<";
		} else if constexpr (std::is_same_v<Comp, std::less_equal<Time>>) {
			os << u8"≤";
		} else if constexpr (std::is_same_v<Comp, std::equal_to<Time>>) {
			os << "=";
		} else if constexpr (std::is_same_v<Comp, std::not_equal_to<Time>>) {
			os << u8"≠";
		} else if constexpr (std::is_same_v<Comp, std::greater_equal<Time>>) {
			os << u8"≥";
		} else if constexpr (std::is_same_v<Comp, std::greater<Time>>) {
			os << ">";
		} else {
			throw InvalidClockComparisonOperatorException();
		}
		os << " " << constraint.comparand_;
		return os;
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

/** Print a ClockConstraint to an ostream
 * @param os The ostream to print to
 * @param constraint The constraint to print
 * @return A reference to the ostream
 */
std::ostream &operator<<(std::ostream &os, const ClockConstraint &constraint);

} // namespace automata
