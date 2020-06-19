/***************************************************************************
 *  ata.h - Alternating Timed Automata
 *
 *  Created: Fri 05 Jun 2020 10:58:27 CEST 10:58
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

#include <ta/ata_formula.h>
#include <ta/automata.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace automata::ata {

/// Thrown if the wrong ATA transition type is attempted
class WrongTransitionTypeException : public std::logic_error
{
public:
	/** Constructor.
	 * @param what The error message
	 */
	WrongTransitionTypeException(const std::string &what) : std::logic_error(what)
	{
	}
};

/// Thrown if a transition with a negative time delta is attempted
class NegativeTimeDeltaException : public std::logic_error
{
public:
	/** Constructor.
	 * @param time_delta The negative time difference to warn about
	 */
	NegativeTimeDeltaException(Time time_delta)
	: std::logic_error(
	  "Cannot do a time transition with negative time delta (=" + std::to_string(time_delta) + ")")
	{
	}
};

/// A transition of an alternating timed automaton
class Transition
{
public:
	friend class AlternatingTimedAutomaton;
	friend bool operator<(const Transition &first, const Transition &second);
	Transition(const Transition &) = delete;
	/** Move constructor */
	Transition(Transition &&) = default;
	/** Constructor.
	 * @param source The source location of the transition
	 * @param symbol The symbol to read with this transition
	 * @param formula The formula that is used to determine the configuration after this transition
	 */
	Transition(const Location &source, const Symbol &symbol, std::unique_ptr<Formula> formula);

private:
	const Location           source_;
	const Symbol             symbol_;
	std::unique_ptr<Formula> formula_;
};

using Configuration = std::set<State>;
using Run           = std::vector<std::pair<std::variant<Symbol, Time>, Configuration>>;
/// An alternating timed automaton.
class AlternatingTimedAutomaton
{
public:
	AlternatingTimedAutomaton() = delete;
	/** Constructor.
	 * @param alphabet The set of symbols used by the automaton
	 * @param initial_location The initial location that determines the initial state
	 * @param final_locations The locations where the automaton is accepting
	 * @param transitions The set of transitions used by the automaton
	 */
	AlternatingTimedAutomaton(const std::set<Symbol> &  alphabet,
	                          const Location &          initial_location,
	                          const std::set<Location> &final_locations,
	                          std::set<Transition>      transitions);
	/** Compute the resulting run after reading a symbol.
	 * @param run The run until now (i.e., the prefix)
	 * @param symbol The symbol to read
	 * @return The run appended with the resulting configuration
	 */
	std::vector<Run> make_symbol_transition(const std::vector<Run> &run, const Symbol &symbol) const;
	/** Compute the resulting run after progressing the time.
	 * @param run The run until now (i.e., the prefix)
	 * @param time The time difference to add to the automaton's clock
	 */
	std::vector<Run> make_time_transition(const std::vector<Run> &run, const Time &time) const;
	/** Check if the ATA accepts a timed word.
	 * @param word The timed word to check
	 * @return true if the given word is accepted
	 */
	[[nodiscard]] bool accepts_word(const TimedWord &word) const;

private:
	std::set<std::set<State>>  get_minimal_models(Formula *formula, ClockValuation v) const;
	const std::set<Symbol>     alphabet_;
	const Location             initial_location_;
	const std::set<Location>   final_locations_;
	const std::set<Transition> transitions_;
};

/** Compare two transitions.
 * @param first The first transition
 * @param second The second transition
 */
bool operator<(const Transition &first, const Transition &second);
} // namespace automata::ata
