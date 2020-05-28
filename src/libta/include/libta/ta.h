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

#include "automata.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace automata {

namespace ta {

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
	Transition(const Location &                                   source,
	           const Symbol &                                     symbol,
	           const Location &                                   target,
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
	const Location                                    source_;
	const Location                                    target_;
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
	std::vector<std::tuple<Symbol, Time, Location>> sequence_;
	std::map<std::string, Clock>                    clock_valuations_;
	Location                                        current_state_;
	Time                                            tick_;
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
	TimedAutomaton(const Location &initial_state, const std::set<Location> &final_states);
	/** Add a state to the TA.
	 * @param state the state to add
	 */
	void add_state(const Location &state);
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
	std::set<Location>       states_;
	const Location           initial_state_;
	const std::set<Location> final_states_;
	// Location                            current_state_;
	// Time                             tick_;
	// std::map<std::string, Clock>     clocks_;
	std::set<std::string>               clocks_;
	std::multimap<Location, Transition> transitions_;
};

/// Compare two paths of a TA
bool operator<(const Path &p1, const Path &p2);

} // namespace ta
} // namespace automata
