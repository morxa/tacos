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

#include "ata_formula.h"
#include "automata.h"

#include <experimental/iterator>
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
	WrongTransitionTypeException(const std::string &what);
};

/// Thrown if a transition with a negative time delta is attempted
class NegativeTimeDeltaException : public std::logic_error
{
public:
	/** Constructor.
	 * @param time_delta The negative time difference to warn about
	 */
	NegativeTimeDeltaException(Time time_delta);
};

template <typename LocationT>
using Configuration = std::set<State<LocationT>>;

template <typename LocationT, typename SymbolT>
using Run = std::vector<std::pair<std::variant<SymbolT, Time>, Configuration<LocationT>>>;

template <typename LocationT, typename SymbolT>
class AlternatingTimedAutomaton;

template <typename LocationT, typename SymbolT>
class Transition;

template <typename LocationT, typename SymbolT>
std::ostream &operator<<(std::ostream &os, const Transition<LocationT, SymbolT> &transition);

template <typename LocationT, typename SymbolT>
std::ostream &operator<<(std::ostream &                                       os,
                         const AlternatingTimedAutomaton<LocationT, SymbolT> &ata);

template <typename LocationT, typename SymbolT>
bool operator<(const Transition<LocationT, SymbolT> &first,
               const Transition<LocationT, SymbolT> &second);

/// A transition of an alternating timed automaton
template <typename LocationT, typename SymbolT>
class Transition
{
public:
	friend class AlternatingTimedAutomaton<LocationT, SymbolT>;
	/** Print a Transition to an ostream
	 * @param os The ostream to print to
	 * @param transition The Transition to print
	 * @return A reference to the ostream
	 */
	// clang-format off
	friend std::ostream &operator<< <>(std::ostream &os, const Transition &transition);
	// clang-format on

	/** Compare two transitions.
	 * @param first The first Transition to compare
	 * @param second The second Transition to compare
	 * @return true if the first Transition is smaller than the second
	 */
	// clang-format off
	friend bool operator< <>(const Transition &first, const Transition &second);
	// clang-format on

	/** Delete the copy constructor, as we cannot copy the formula pointer. */
	Transition(const Transition &) = delete;

	/** Move constructor */
	Transition(Transition &&) = default;

	/** Constructor.
	 * @param source The source location of the transition
	 * @param symbol The symbol to read with this transition
	 * @param formula The formula that is used to determine the configuration after this transition
	 */
	Transition(const LocationT &                   source,
	           const SymbolT &                     symbol,
	           std::unique_ptr<Formula<LocationT>> formula);

public:
	/// The source location of the transition
	const LocationT source_;
	/// The symbol this transition can fire on
	const SymbolT symbol_;

private:
	std::unique_ptr<Formula<LocationT>> formula_;
};

/// An alternating timed automaton.
template <typename LocationT, typename SymbolT>
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
	AlternatingTimedAutomaton(const std::set<SymbolT> &                alphabet,
	                          const LocationT &                        initial_location,
	                          const std::set<LocationT> &              final_locations,
	                          std::set<Transition<LocationT, SymbolT>> transitions);

	/** Get the initial configuration.
	 * @return The initial configuration of the automaton.
	 */
	[[nodiscard]] Configuration<LocationT> get_initial_configuration() const;

	/** Compute the resulting configurations after making a symbol step.
	 * @param start_states The starting configuration
	 * @param symbol The symbol to read
	 * @return The configurations after making the symbol step
	 */
	std::set<Configuration<LocationT>> make_symbol_step(const Configuration<LocationT> &start_states,
	                                                    const SymbolT &                 symbol) const;

	/** Compute the resulting run after reading a symbol.
	 * @param runs The valid runs resulting from reading previous symbols
	 * @param symbol The symbol to read
	 * @return The run appended with the resulting configuration
	 */
	std::vector<Run<LocationT, SymbolT>>
	make_symbol_transition(const std::vector<Run<LocationT, SymbolT>> &runs,
	                       const SymbolT &                             symbol) const;

	/** Compute the resulting configuration after progressing the time.
	 * @param start The starting configuration
	 * @param time The time difference to add to the automaton's clock
	 * @return The configuration after making the time step
	 */
	Configuration<LocationT> make_time_step(const Configuration<LocationT> &start,
	                                        const Time &                    time) const;

	/** Compute the resulting run after progressing the time.
	 * @param runs The valid runs resulting from reading previous symbols
	 * @param time The time difference to add to the automaton's clock
	 * @return The run augmented with the new configuration
	 */
	std::vector<Run<LocationT, SymbolT>>
	make_time_transition(const std::vector<Run<LocationT, SymbolT>> &runs, const Time &time) const;

	/** Check if the given configuration is accepting.
	 * @param configuration The configuration to check
	 * @return true if the configuration is an accepting configuration of the ATA
	 */
	[[nodiscard]] bool
	is_accepting_configuration(const Configuration<LocationT> &configuration) const;

	/** Check if the ATA accepts a timed word.
	 * @param word The timed word to check
	 * @return true if the given word is accepted
	 */
	[[nodiscard]] bool accepts_word(const TimedWord &word) const;

	/** Print an AlternatingTimedAutomaton to an ostream
	 * @param os The ostream to print to
	 * @param ata The AlternatingTimedAutomaton to print
	 * @return A reference to the ostream
	 */
	// clang-format off
	friend std::ostream &
	operator<< <>(std::ostream &os, const AlternatingTimedAutomaton &ata);
	// clang-format on

private:
	std::set<std::set<State<LocationT>>> get_minimal_models(Formula<LocationT> *formula,
	                                                        ClockValuation      v) const;

	const std::set<SymbolT>                        alphabet_;
	const LocationT                                initial_location_;
	const std::set<LocationT>                      final_locations_;
	const std::set<Transition<LocationT, SymbolT>> transitions_;
};

extern template class Transition<std::string, std::string>;
extern template class AlternatingTimedAutomaton<std::string, std::string>;

} // namespace automata::ata

/** Print a configuration to an ostream.
 * @param os The ostream to print to
 * @param configuration The configuration to print
 * @return A reference to the ostream
 */
template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const automata::ata::Configuration<LocationT> &configuration)
{
	os << "{ ";
	bool first = true;
	for (const auto &state : configuration) {
		if (!first) {
			os << ", ";
		} else {
			first = false;
		}
		os << state;
	}
	os << " }";
	return os;
}

/** Print a run to an ostream.
 * @param os The ostream to print to
 * @param run The run to print
 * @return A reference to the ostream
 */
template <typename LocationT, typename SymbolT>
std::ostream &
operator<<(std::ostream &os, const automata::ata::Run<LocationT, SymbolT> &run)
{
	for (const auto &[step, configuration] : run) {
		// simple arrow for symbol step, dashed arrow for time step
		const std::string arrow = step.index() == 0 ? u8"→" : u8"⇢";
		os << " " << arrow << " ";
		std::visit([&os](const auto &s) { os << s; }, step);
		os << " " << arrow << " ";
		os << configuration;
	}
	return os;
}

#include "ata.hpp"
