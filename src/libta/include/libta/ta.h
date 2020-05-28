/***************************************************************************
 *  ta.h - Core functionality for timed automata
 *
 *  Created: Tue 26 May 2020 13:44:41 CEST 13:44
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

#include <algorithm>
#include <compare>
#include <exception>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace ta {

using State     = std::string;
using Symbol    = std::string;
using Time      = double;
using Endpoint  = unsigned int;
using TimedWord = std::vector<std::pair<Symbol, Time>>;

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

/// Invalid state encountered
/*** This exception is thrown when some state (e.g., as part of a transition) is not  part of a
 * timed automaton.
 */
class InvalidStateException : public std::invalid_argument
{
public:
	/** Constructor
	 * @param state The name of the invalid state
	 */
	explicit InvalidStateException(const State &state)
	: std::invalid_argument("Invalid state: " + state)
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
                                     AtomicClockConstraintT<std::greater_equal<Time>>,
                                     AtomicClockConstraintT<std::greater<Time>>>;

/// A transition in a timed automaton.
/** @see TimedAutomaton
 */
class Transition
{
public:
	friend class TimedAutomaton;
	/** Constructor.
	 * @param source the source state
	 * @param symbol the symbol to read with this transition
	 * @param target the target state
	 * @param clock_constraints A map defining the constraints of the clock,
	 *        where the key specifies the name of the clock and the value is a
	 *        constraint on that clock
	 * @param clock_resets the set of clocks to reset on this transition
	 */
	Transition(const State &                                      source,
	           const Symbol &                                     symbol,
	           const State &                                      target,
	           const std::multimap<std::string, ClockConstraint> &clock_constraints = {},
	           const std::set<std::string> &                      clock_resets      = {});
	/** Check whether the transition is enabled on the given symbol and clock valuations.
	 * More specifically, check if the given symbol matches this transition's symbol, and that the
	 * clock valuations satisfy all clock constraints.
	 * @param symbol The symbol to check
	 * @param clock_vals The clock valuations, given as map with the clock names as keys and the
	 * clocks as value
	 * @return true if the transition can be taken.
	 */
	bool is_enabled(const Symbol &symbol, const std::map<std::string, Clock> &clock_vals) const;

private:
	const State                                       source_;
	const State                                       target_;
	const Symbol                                      symbol_;
	const std::multimap<std::string, ClockConstraint> clock_constraints_;
	const std::set<std::string>                       clock_resets_;
};

/// One specific (finite) path in the timed automaton.
class Path
{
public:
	friend class TimedAutomaton;
	friend bool operator<(const Path &p1, const Path &p2);
	/// Constructor
	/** Start a new path in the given initial state with the given clocks.
	 * @param initial_state the initial state of the path, should be the same as the TA's initial
	 * state
	 * @param clocks a set of clock names, should be names of the TA's clocks
	 */
	Path(std::string initial_state, std::set<std::string> clocks);

private:
	std::vector<std::tuple<Symbol, Time, State>> sequence_;
	std::map<std::string, Clock>                 clock_valuations_;
	State                                        current_state_;
	Time                                         tick_;
};

/// A timed automaton.
/** A TimedAutomaton consists of a set of states, an initial state, a final state, a set of
 * clocks, and a set of transitions. A simple timed automaton with two states and a single
 * transition without constraints can be constructed with
 * @code
 * TimedAutomaton ta{"s0", {"s1"}};
 * ta.add_transition(Transition("s0", "a", "s1"));
 * @endcode
 * To construct a timed automaton with a clock constraint <tt>x < 1</tt>, use
 * @code
 * TimedAutomaton ta{"s0", {"s1"}};
 * ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(1);
 * ta.add_transition(Transition("s0", "a", "s1", {{"x", c}}));
 * @endcode
 */
class TimedAutomaton
{
public:
	TimedAutomaton() = delete;
	/** Constructor.
	 * @param initial_state the initial state
	 * @param final_states a set of final states
	 */
	TimedAutomaton(const State &initial_state, const std::set<State> &final_states);
	/** Add a state to the TA.
	 * @param state the state to add
	 */
	void add_state(const State &state);
	/** Add a clock to the TA.
	 * @param name the name of the clock
	 */
	void add_clock(const std::string &name);
	/** Add a set of states to the TA
	 * @param states the states to add
	 */
	void add_states(const std::set<std::string> &states);
	/** Add a transition to the TA.
	 * @param transition The transition to add, must only mention clocks and states that are already
	 * part of the TA.
	 */
	void add_transition(const Transition &transition);
	/// Let the TA make a transition on the given symbol at the given time.
	/** Check if there is a transition that can be enabled on the given symbol at the given time,
	 * starting with the given path. If so, modify the given path, i.e., apply the transition by
	 * switching to the new state, increasing all clocks by the time difference, and resetting all
	 * clocks specified in the transition. This always uses the first transition that is enabled,
	 * i.e., it does not work properly on non-deterministic TAs.
	 * @param path The path prefix to start at
	 * @param symbol The symbol to read
	 * @param time The (absolute) time associated with the symbol
	 * @return a (possibly empty) set of valid paths after applying the transition
	 */
	std::set<Path> make_transition(Path path, const Symbol &symbol, const Time &time) const;
	/// Check if the TA accepts the given timed word.
	/** Iteratively apply transitions for each (symbol,time) pair in the given timed word.
	 * @param word the word to read
	 * @return true if the word was accepted
	 */
	bool accepts_word(const TimedWord &word) const;

private:
	std::set<State>       states_;
	const State           initial_state_;
	const std::set<State> final_states_;
	// State                            current_state_;
	// Time                             tick_;
	// std::map<std::string, Clock>     clocks_;
	std::set<std::string>            clocks_;
	std::multimap<State, Transition> transitions_;
};

/// Compare two paths of a TA
bool operator<(const Path &p1, const Path &p2);

} // namespace ta
