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

#ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_AUTOMATA_H
#define SRC_AUTOMATA_INCLUDE_AUTOMATA_AUTOMATA_H

#include "utilities/types.h"

#include <boost/format.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace tacos::automata {

template <typename Symbol>
using TimedWord = std::vector<std::pair<Symbol, Time>>;

template <typename Comp>
class AtomicClockConstraintT;

using ClockConstraint = std::variant<AtomicClockConstraintT<std::less<Time>>,
                                     AtomicClockConstraintT<std::less_equal<Time>>,
                                     AtomicClockConstraintT<std::equal_to<Time>>,
                                     AtomicClockConstraintT<std::not_equal_to<Time>>,
                                     AtomicClockConstraintT<std::greater_equal<Time>>,
                                     AtomicClockConstraintT<std::greater<Time>>>;

template <class Comp>
std::ostream &operator<<(std::ostream &                                os,
                         const automata::AtomicClockConstraintT<Comp> &constraint);

/** Print a ClockConstraint to an ostream
 * @param os The ostream to print to
 * @param constraint The constraint to print
 * @return A reference to the ostream
 */
std::ostream &operator<<(std::ostream &os, const automata::ClockConstraint &constraint);

/** Print a multimap of ClockConstraints to an ostream
 * @param os The ostream to print to
 * @param constraints The constraints to print
 * @return A reference to the ostream
 */
std::ostream &operator<<(std::ostream &                                               os,
                         const std::multimap<std::string, automata::ClockConstraint> &constraints);

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
	: std::invalid_argument((boost::format("Invalid clock: %1%") % clock_name).str())
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
template <typename Symbol>
class InvalidSymbolException : public std::invalid_argument
{
public:
	/** Constructor */
	explicit InvalidSymbolException(const Symbol &symbol)
	: std::invalid_argument("Invalid symbol '" + std::string(symbol) + "'")
	{
	}
};

template <class Comp>
class AtomicClockConstraintT;

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
	// clang-format off
	friend std::ostream &operator<< <>(std::ostream &os, const AtomicClockConstraintT &constraint);
	// clang-format on

private:
	const Endpoint comparand_;
};

bool is_satisfied(const ClockConstraint &constraint, const ClockValuation &valuation);

inline std::optional<std::size_t>
get_relation_index(const ClockConstraint &constraint)
{
	if (std::holds_alternative<AtomicClockConstraintT<std::less<Time>>>(constraint)) {
		return 0;
	} else if (std::holds_alternative<AtomicClockConstraintT<std::less_equal<Time>>>(constraint)) {
		return 1;
	} else if (std::holds_alternative<AtomicClockConstraintT<std::equal_to<Time>>>(constraint)) {
		return 2;
	} else if (std::holds_alternative<AtomicClockConstraintT<std::not_equal_to<Time>>>(constraint)) {
		return 3;
	} else if (std::holds_alternative<AtomicClockConstraintT<std::greater_equal<Time>>>(constraint)) {
		return 4;
	} else if (std::holds_alternative<AtomicClockConstraintT<std::greater<Time>>>(constraint)) {
		return 5;
	} else {
		assert(false);
		return std::nullopt;
	}
}

inline bool
operator<(const ClockConstraint &lhs, const ClockConstraint &rhs)
{
	auto lhs_idx = get_relation_index(lhs);
	auto rhs_idx = get_relation_index(rhs);
	if (!lhs_idx || !rhs_idx) {
		throw std::logic_error("Unknown relation symbol in constraints cannot be handled.");
	}
	if (lhs_idx.value() < rhs_idx.value()) {
		return true;
	} else if (lhs_idx.value() == rhs_idx.value()) {
		Time lhs_time = std::visit([](const auto &atomic_clock_constraint)
		                             -> Time { return atomic_clock_constraint.get_comparand(); },
		                           lhs);
		Time rhs_time = std::visit([](const auto &atomic_clock_constraint)
		                             -> Time { return atomic_clock_constraint.get_comparand(); },
		                           rhs);

		return lhs_time < rhs_time;
	} else {
		return false;
	}
}

} // namespace tacos::automata

#include "automata.hpp"

#endif /* ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_AUTOMATA_H */
