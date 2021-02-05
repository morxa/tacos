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
#include <variant>
#include <vector>

namespace automata {

namespace ta {

template <typename LocationT>
using Configuration = std::pair<LocationT, ClockSetValuation>;

template <typename LocationT, typename AP>
class TimedAutomaton;

/// A transition in a timed automaton.
/** @see TimedAutomaton
 */
template <typename LocationT, typename AP>
class Transition
{
public:
	friend class TimedAutomaton<LocationT, AP>;
	/// Compare two transitions.
	/** Two transitions are equal if they use the same source, target, read the
	 * same symbol, have the same clock constraints, and reset the same clocks.
	 * @param lhs The left-hand side Transition
	 * @param rhs The right-hand side Transition
	 */
	friend bool
	operator==(const Transition<LocationT, AP> &lhs, const Transition<LocationT, AP> &rhs)
	{
		return lhs.source_ == rhs.source_ && lhs.target_ == rhs.target_ && lhs.symbol_ == rhs.symbol_
		       && lhs.clock_constraints_ == rhs.clock_constraints_
		       && lhs.clock_resets_ == rhs.clock_resets_;
	}
	/** Constructor.
	 * @param source the source location
	 * @param symbol the symbol to read with this transition
	 * @param target the target location
	 * @param clock_constraints A map defining the constraints of the clock,
	 *        where the key specifies the name of the clock and the value is a
	 *        constraint on that clock
	 * @param clock_resets the set of clocks to reset on this transition
	 */
	Transition(const LocationT &                                        source,
	           const AP &                                               symbol,
	           const LocationT &                                        target,
	           const std::multimap<std::string, const ClockConstraint> &clock_constraints = {},
	           const std::set<std::string> &                            clock_resets      = {})
	: source_(source),
	  target_(target),
	  symbol_(symbol),
	  clock_constraints_(clock_constraints),
	  clock_resets_(clock_resets)
	{
	}
	/** Check whether the transition is enabled on the given symbol and clock valuations.
	 * More specifically, check if the given symbol matches this transition's symbol, and that the
	 * clock valuations satisfy all clock constraints.
	 * @param symbol The symbol to check
	 * @param clock_vals The clock valuations, given as map with the clock names as keys and the
	 * clocks as value
	 * @return true if the transition can be taken.
	 */
	bool
	is_enabled(const AP &symbol, const ClockSetValuation &clock_vals) const
	{
		if (symbol != symbol_) {
			return false;
		}
		return std::all_of(std::begin(clock_constraints_),
		                   std::end(clock_constraints_),
		                   [&clock_vals](const auto &constraint) {
			                   return is_satisfied(constraint.second, clock_vals.at(constraint.first));
		                   });
	}

	/**
	 * @brief Get the clock constraints defining the guard conditions
	 *
	 * @return const std::multimap<std::string, const ClockConstraint>&
	 */
	const std::multimap<std::string, const ClockConstraint> &
	get_guards() const
	{
		return clock_constraints_;
	}

	const LocationT                                         source_;            ///< source location
	const LocationT                                         target_;            ///< target location
	const AP                                                symbol_;            ///< transition label
	const std::multimap<std::string, const ClockConstraint> clock_constraints_; ///< guards
	const std::set<std::string>                             clock_resets_;      ///< resets
};

/// One specific (finite) path in the timed automaton.
template <typename LocationT, typename AP>
class Path
{
public:
	friend class TimedAutomaton<LocationT, AP>;
	/// Compare two paths of a TA
	friend bool
	operator<(const Path<LocationT, AP> &p1, const Path<LocationT, AP> &p2)
	{
		return p1.sequence_ < p2.sequence_;
	}
	/// Constructor
	/** Start a new path in the given initial location with the given clocks.
	 * @param initial_location the initial location of the path, should be the same as the TA's
	 * initial location
	 * @param clocks a set of clock names, should be names of the TA's clocks
	 */
	Path(LocationT initial_location, std::set<std::string> clocks)
	: current_location_(initial_location), tick_(0)
	{
		for (const auto &clock : clocks) {
			clock_valuations_.emplace(std::make_pair(clock, Clock()));
		}
	}

	/** Get the current configuration of the path.
	 * The current configuration is the last configuration reached.
	 * @return The current configuration of the path
	 */
	Configuration<LocationT>
	get_current_configuration() const
	{
		return std::make_pair(current_location_, clock_valuations_);
	}

private:
	std::vector<std::tuple<AP, Time, LocationT>> sequence_;
	std::map<std::string, Clock>                 clock_valuations_;
	LocationT                                    current_location_;
	Time                                         tick_;
};

/// A timed automaton.
/** A TimedAutomaton consists of a set of locations, an initial location, a final location, a set of
 * clocks, and a set of transitions. A simple timed automaton with two locations and a single
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
template <typename LocationT, typename AP>
class TimedAutomaton
{
public:
	TimedAutomaton() = delete;
	/** Constructor.
	 * @param alphabet The valid symbols in the TimedAutomaton
	 * @param initial_location the initial location
	 * @param final_locations a set of final locations
	 */
	TimedAutomaton(const std::set<AP> &       alphabet,
	               const LocationT &          initial_location,
	               const std::set<LocationT> &final_locations)
	: alphabet_(alphabet),
	  locations_{initial_location},
	  initial_location_(initial_location),
	  final_locations_(final_locations)
	{
		add_locations(final_locations_);
	}

	/** Get the alphabet
	 * @return A reference to the set of symbols used by the TimedAutomaton
	 */
	const std::set<AP> &
	get_alphabet() const
	{
		return alphabet_;
	}

	/** Add a location to the TA.
	 * @param location the location to add
	 */
	void
	add_location(const LocationT &location)
	{
		locations_.insert(location);
	}
	/** Add a clock to the TA.
	 * @param name the name of the clock
	 */
	void
	add_clock(const std::string &name)
	{
		clocks_.insert(name);
	}
	/** Add a set of locations to the TA
	 * @param locations the locations to add
	 */
	void
	add_locations(const std::set<LocationT> &locations)
	{
		for (const auto &location : locations) {
			add_location(location);
		}
	}
	/** Add a transition to the TA.
	 * @param transition The transition to add, must only mention clocks and locations that are
	 * already part of the TA.
	 */
	void
	add_transition(const Transition<LocationT, AP> &transition)
	{
		if (alphabet_.count(transition.symbol_) == 0) {
			throw InvalidSymbolException(transition.symbol_);
		}
		if (!locations_.count(transition.source_)) {
			throw InvalidLocationException(transition.source_);
		}
		if (!locations_.count(transition.target_)) {
			throw InvalidLocationException(transition.target_);
		}
		for (const auto &[clock_name, constraint] : transition.clock_constraints_) {
			if (!clocks_.count(clock_name)) {
				throw InvalidClockException(clock_name);
			};
		}
		for (const auto &clock_name : transition.clock_resets_) {
			if (!clocks_.count(clock_name)) {
				throw InvalidClockException(clock_name);
			};
		}
		transitions_.insert({transition.source_, transition});
	}

	/** Compute the resulting configuration after making a symbol step.
	 */
	std::set<Configuration<LocationT>>
	make_symbol_step(const Configuration<LocationT> &configuration, const AP &symbol) const
	{
		std::set<Configuration<LocationT>> res;
		// TODO This may cause an issue if the transitions are not sorted as expected, because
		// equal_range returns a sub-sequence
		auto [first, last] = transitions_.equal_range(configuration.first);
		while (first != last) {
			auto trans = std::find_if(first, last, [&](const auto &trans) {
				return trans.second.is_enabled(symbol, configuration.second);
			});
			if (trans == last) {
				return res;
			}
			first                         = std::next(trans);
			ClockSetValuation next_clocks = configuration.second;
			for (const auto &name : trans->second.clock_resets_) {
				next_clocks[name].reset();
			}
			res.insert(Configuration<LocationT>(trans->second.target_, next_clocks));
		}
		return res;
	}
	/// Let the TA make a transition on the given symbol at the given time.
	/** Check if there is a transition that can be enabled on the given symbol at the given time,
	 * starting with the given path. If so, modify the given path, i.e., apply the transition by
	 * switching to the new location, increasing all clocks by the time difference, and resetting all
	 * clocks specified in the transition. This always uses the first transition that is enabled,
	 * i.e., it does not work properly on non-deterministic TAs.
	 * @param path The path prefix to start at
	 * @param symbol The symbol to read
	 * @param time The (absolute) time associated with the symbol
	 * @return a (possibly empty) set of valid paths after applying the transition
	 */
	std::set<Path<LocationT, AP>>
	make_transition(Path<LocationT, AP> path, const AP &symbol, const Time &time) const
	{
		if (path.tick_ > time) {
			return {};
		}
		for (auto &[name, clock] : path.clock_valuations_) {
			clock.tick(time - path.tick_);
		}
		path.tick_ = time;
		std::set<Path<LocationT, AP>> paths;
		Configuration<LocationT>      start_configuration = path.get_current_configuration();
		for (const auto &target_configuration : make_symbol_step(start_configuration, symbol)) {
			auto new_path = path;
			path.sequence_.emplace_back(symbol, time, target_configuration.first);
			new_path.current_location_ = target_configuration.first;
			new_path.clock_valuations_ = target_configuration.second;
			paths.insert(new_path);
		}
		return paths;
	}
	/// Check if the TA accepts the given timed word.
	/** Iteratively apply transitions for each (symbol,time) pair in the given timed word.
	 * @param word the word to read
	 * @return true if the word was accepted
	 */
	bool
	accepts_word(const TimedWord &word) const
	{
		std::set<Path<LocationT, AP>> paths{Path<LocationT, AP>{initial_location_, clocks_}};
		for (auto &[symbol, time] : word) {
			std::set<Path<LocationT, AP>> res_paths;
			for (auto &path : paths) {
				auto new_paths = make_transition(path, symbol, time);
				res_paths.insert(std::begin(new_paths), std::end(new_paths));
			}
			paths = res_paths;
			if (paths.empty()) {
				return false;
			}
		}
		for (auto &path : paths) {
			if (final_locations_.find(path.current_location_) != final_locations_.end()) {
				return true;
			};
		}
		return false;
	}

	/// Get the enabled transitions in a given configuration.
	std::vector<Transition<LocationT, AP>>
	get_enabled_transitions(const Configuration<LocationT> &configuration)
	{
		std::vector<Transition<LocationT, AP>> res;
		for (const auto &symbol : alphabet_) {
			for (const auto &[source, transition] : transitions_) {
				if (source == configuration.first && transition.is_enabled(symbol, configuration.second)) {
					res.push_back(transition);
				}
			}
		}
		return res;
	}

	Time
	get_largest_constant() const
	{
		Time res{0};
		for (const auto &[symbol, transition] : transitions_) {
			for (const auto &[symbol, constraint] : transition.get_guards()) {
				Time candidate = std::visit([](const auto &c) { return c.get_comparand(); }, constraint);
				if (candidate > res) {
					res = candidate;
				}
			}
		}
		return res;
	}

private:
	std::set<AP>                                        alphabet_;
	std::set<LocationT>                                 locations_;
	const LocationT                                     initial_location_;
	const std::set<LocationT>                           final_locations_;
	std::set<std::string>                               clocks_;
	std::multimap<LocationT, Transition<LocationT, AP>> transitions_;
};

///// Compare two paths of a TA
// template <typename LocationT>
// bool
// operator<(const Path<LocationT> &p1, const Path<LocationT> &p2)
//{
//	return p1.sequence_ < p2.sequence_;
//}

} // namespace ta
} // namespace automata

template <typename Location>
std::ostream &
operator<<(std::ostream &os, const automata::ta::Configuration<Location> &configuration)
{
	os << "(" << configuration.first << ", ";
	std::for_each(configuration.second.begin(),
	              configuration.second.end(),
	              [&os](const auto &kvPair) {
		              os << "(" << kvPair.first << ": " << kvPair.second << ")";
	              });
	os << ")";
	return os;
}
