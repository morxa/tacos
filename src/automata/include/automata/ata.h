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

template <typename LocationT, typename SymbolT>
class AlternatingTimedAutomaton;

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
	friend std::ostream &
	operator<<(std::ostream &os, const Transition &transition)
	{
		os << transition.source_ << u8" → " << transition.symbol_ << u8" → " << *transition.formula_;
		return os;
	}

	/** Compare two transitions.
	 * @param first The first Transition to compare
	 * @param second The second Transition to compare
	 * @return true if the first Transition is smaller than the second
	 */
	friend bool
	operator<(const Transition &first, const Transition &second)
	{
		if (first.source_ != second.source_) {
			return first.source_ < second.source_;
		} else if (first.symbol_ != second.symbol_) {
			return first.symbol_ < second.symbol_;
		} else {
			return first.formula_ < second.formula_;
		}
	}
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
	           std::unique_ptr<Formula<LocationT>> formula)
	: source_(source), symbol_(symbol), formula_(std::move(formula))
	{
	}

public:
	/// The source location of the transition
	const LocationT source_;
	/// The symbol this transition can fire on
	const SymbolT symbol_;

private:
	std::unique_ptr<Formula<LocationT>> formula_;
};

template <typename LocationT>
using Configuration = std::set<State<LocationT>>;

template <typename LocationT, typename SymbolT>
using Run = std::vector<std::pair<std::variant<SymbolT, Time>, Configuration<LocationT>>>;

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
	                          std::set<Transition<LocationT, SymbolT>> transitions)
	: alphabet_(alphabet),
	  initial_location_(initial_location),
	  final_locations_(final_locations),
	  transitions_(std::move(transitions))
	{
	}

	/** Compute the resulting configurations after making a symbol step.
	 * @param start_states The starting configuration
	 * @param symbol The symbol to read
	 * @return The configurations after making the symbol step
	 */
	std::set<Configuration<LocationT>>
	make_symbol_step(const Configuration<LocationT> &start_states, const SymbolT &symbol) const
	{
		// A vector of a set of target configurations that are reached when following a transition.
		// One entry for each start state
		std::vector<std::set<Configuration<LocationT>>> models;
		// If the start states are empty, we know that the empty set of states is
		// a minimal model of the last transition step.
		if (start_states.empty()) {
			models = {{{}}};
		}
		for (const auto &state : start_states) {
			auto t = std::find_if(transitions_.cbegin(), transitions_.cend(), [&](const auto &t) {
				return t.source_ == state.first && t.symbol_ == symbol;
			});
			if (t == transitions_.end()) {
				continue;
			}
			const auto new_states = get_minimal_models(t->formula_.get(), state.second);
			models.push_back(new_states);
		}
		// We were not able to make any transition.
		if (models.empty()) {
			return {};
		}
		// The resulting configurations after computing the cartesian product of all target
		// configurations of each state
		std::set<Configuration<LocationT>> configurations;
		// The vector models now contains in each element all minimal models for one start state.
		// Transform the vector such that in each entry, we have one minimal model
		// for each start state.
		//
		// Populate the configurations by splitting the first configuration in all minimal models
		// { { m1, m2, m3 } } -> { { m1 }, { m2 }, { m3 } }
		ranges::for_each(models[0],
		                 [&](const auto &state_model) { configurations.insert({state_model}); });
		// Add models from the other configurations
		std::for_each(std::next(models.begin()), models.end(), [&](const auto &state_models) {
			std::set<Configuration<LocationT>> expanded_configurations;
			ranges::for_each(state_models, [&](const auto &state_model) {
				ranges::for_each(configurations, [&](const auto &configuration) {
					auto expanded_configuration = configuration;
					expanded_configuration.insert(state_model.begin(), state_model.end());
					expanded_configurations.insert(expanded_configuration);
				});
			});
			configurations = expanded_configurations;
		});
		return configurations;
	}

	/** Compute the resulting run after reading a symbol.
	 * @param runs The valid runs resulting from reading previous symbols
	 * @param symbol The symbol to read
	 * @return The run appended with the resulting configuration
	 */
	std::vector<Run<LocationT, SymbolT>>
	make_symbol_transition(const std::vector<Run<LocationT, SymbolT>> &runs,
	                       const SymbolT &                             symbol) const
	{
		std::vector<Run<LocationT, SymbolT>> res;
		for (const auto &run : runs) {
			if (!run.empty() && std::holds_alternative<SymbolT>(run.back().first)) {
				throw WrongTransitionTypeException(
				  "Cannot do two subsequent symbol transitions, transitions must be "
				  "alternating between symbol and time");
			}
			std::set<State<LocationT>> start_states;
			if (run.empty()) {
				start_states = {std::make_pair(initial_location_, 0)};
			} else {
				start_states = run.back().second;
			}
			auto configurations = make_symbol_step(start_states, symbol);
			ranges::for_each(configurations, [&](const auto &c) {
				auto expanded_run = run;
				expanded_run.push_back(std::make_pair(std::variant<SymbolT, Time>(symbol), c));
				res.push_back(expanded_run);
			});
		}
		return res;
	}

	/** Compute the resulting configuration after progressing the time.
	 * @param start The starting configuration
	 * @param time The time difference to add to the automaton's clock
	 * @return The configuration after making the time step
	 */
	Configuration<LocationT>
	make_time_step(const Configuration<LocationT> &start, const Time &time) const
	{
		if (time < 0) {
			throw NegativeTimeDeltaException(time);
		}
		Configuration<LocationT> res;
		std::transform(start.cbegin(),
		               start.cend(),
		               std::inserter(res, res.begin()),
		               [&time](const auto &s) { return std::make_pair(s.first, s.second + time); });
		return res;
	}

	/** Compute the resulting run after progressing the time.
	 * @param runs The valid runs resulting from reading previous symbols
	 * @param time The time difference to add to the automaton's clock
	 * @return The run augmented with the new configuration
	 */
	std::vector<Run<LocationT, SymbolT>>
	make_time_transition(const std::vector<Run<LocationT, SymbolT>> &runs, const Time &time) const
	{
		std::vector<Run<LocationT, SymbolT>> res;
		for (auto run : runs) {
			if (run.empty()) {
				throw WrongTransitionTypeException(
				  "Cannot do a time transition on empty run, a run must start with a symbol transition");
			}
			if (std::holds_alternative<Time>(run.back().first)) {
				throw WrongTransitionTypeException(
				  "Cannot do two subsequent time transitions, transitions must be "
				  "alternating between symbol and time");
			}
			run.push_back(std::make_pair(time, make_time_step(run.back().second, time)));
			res.push_back(run);
		}
		return res;
	}
	/** Check if the ATA accepts a timed word.
	 * @param word The timed word to check
	 * @return true if the given word is accepted
	 */
	[[nodiscard]] bool
	accepts_word(const TimedWord &word) const
	{
		if (word.size() == 0) {
			return false;
		}
		if (word[0].second != 0) {
			throw InvalidTimedWordException("Invalid time initialization "
			                                + std::to_string(word[0].second)
			                                + " in timed word, must be 0");
		}
		std::vector<Run<LocationT, SymbolT>> runs = {{}};
		// A run on a word (a0,t0), (a1,t1) is defined as the sequence from making the transitions
		// C0 ->[a0] C1 ->[t1-t0] C1 ->[a1] C2.
		// Note how it operates on the time difference to the *next* timed symbol.
		// Thus, we need to read the first symbol and initialize last_time.
		const auto &[symbol, time] = *word.begin();
		runs                       = make_symbol_transition(runs, symbol);
		Time last_time             = time;
		std::for_each(std::next(word.begin()), word.end(), [&](const auto &timed_symbol) {
			const auto &[symbol, time] = timed_symbol;
			runs                       = make_time_transition(runs, time - last_time);
			last_time                  = time;
			runs                       = make_symbol_transition(runs, symbol);
		});
		// There must be one run...
		return std::any_of(runs.begin(), runs.end(), [this](const auto &run) {
			// ... where the final configuration ...
			auto final_configuration = run.back().second;
			// ... only consists of accepting locations.
			return std::all_of(final_configuration.begin(),
			                   final_configuration.end(),
			                   [this](auto const &state) {
				                   return final_locations_.count(state.first) > 0;
			                   });
		});
	}

	/** Print an AlternatingTimedAutomaton to an ostream
	 * @param os The ostream to print to
	 * @param ata The AlternatingTimedAutomaton to print
	 * @return A reference to the ostream
	 */
	friend std::ostream &
	operator<<(std::ostream &os, const AlternatingTimedAutomaton &ata)
	{
		os << "Alphabet: {";
		std::copy(ata.alphabet_.begin(),
		          ata.alphabet_.end(),
		          std::experimental::make_ostream_joiner(os, ", "));
		os << "}";
		os << ", initial location: " << ata.initial_location_;
		os << ", final locations: {";
		std::copy(ata.final_locations_.begin(),
		          ata.final_locations_.end(),
		          std::experimental::make_ostream_joiner(os, ", "));
		os << "}";
		os << ", transitions:";
		for (const auto &transition : ata.transitions_) {
			os << '\n' << "  " << transition;
		}

		return os;
	}

private:
	std::set<std::set<State<LocationT>>>
	get_minimal_models(Formula<LocationT> *formula, ClockValuation v) const
	{
		return formula->get_minimal_models(v);
	}

	const std::set<SymbolT>                        alphabet_;
	const LocationT                                initial_location_;
	const std::set<LocationT>                      final_locations_;
	const std::set<Transition<LocationT, SymbolT>> transitions_;
};

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
