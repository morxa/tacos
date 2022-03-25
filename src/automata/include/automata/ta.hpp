/***************************************************************************
 *  ta.hpp - Core functionality for timed automata
 *
 *  Created:   Mon 15 Feb 15:09:10 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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
 *  Read the full text in the LICENSE.md file.
 */

#pragma once

#include "ta.h"

#include <algorithm>
#include <iterator>

namespace tacos::automata::ta {

template <typename LocationT, typename AP>
std::ostream &
operator<<(std::ostream &                                                       os,
           const std::multimap<Location<LocationT>, Transition<LocationT, AP>> &transitions)
{
	for (const auto &[source, transition] : transitions) {
		os << transition << '\n';
	}
	return os;
}

template <typename Location>
std::ostream &
operator<<(std::ostream &os, const Configuration<Location> &configuration)
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

template <typename T>
std::ostream &
operator<<(std::ostream &os, const std::set<T> &elements)
{
	if (elements.empty()) {
		os << "{}";
		return os;
	}
	bool first = true;
	os << "{ ";
	for (const auto &element : elements) {
		if (first) {
			first = false;
		} else {
			os << ", ";
		}
		os << element;
	}
	os << " }";
	return os;
}

template <typename LocationT, typename AP>
std::ostream &
operator<<(std::ostream &os, const Transition<LocationT, AP> &transition)
{
	os << transition.source_ << u8" → " << transition.symbol_ << " / "
	   << transition.clock_constraints_ << " / " << transition.clock_resets_ << u8" → "
	   << transition.target_;
	return os;
}

template <typename LocationT, typename AP>
std::ostream &
operator<<(std::ostream &os, const TimedAutomaton<LocationT, AP> &ta)
{
	os << "Alphabet: " << ta.alphabet_ << ", initial location: " << ta.initial_location_
	   << ", final locations: " << ta.final_locations_ << ", transitions:\n"
	   << ta.transitions_;
	return os;
}

template <typename LocationT, typename AP>
bool
Transition<LocationT, AP>::is_enabled(const AP &symbol, const ClockSetValuation &clock_vals) const
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

template <typename LocationT, typename AP>
bool
operator==(const Transition<LocationT, AP> &lhs, const Transition<LocationT, AP> &rhs)
{
	return lhs.source_ == rhs.source_ && lhs.target_ == rhs.target_ && lhs.symbol_ == rhs.symbol_
	       && lhs.clock_constraints_ == rhs.clock_constraints_
	       && lhs.clock_resets_ == rhs.clock_resets_;
}

template <typename LocationT, typename AP>
bool
operator<(const Transition<LocationT, AP> &lhs, const Transition<LocationT, AP> &rhs)
{
	// cheap checks first
	auto left_tie  = std::tie(lhs.source_, lhs.target_, lhs.symbol_, lhs.clock_resets_);
	auto right_tie = std::tie(rhs.source_, rhs.target_, rhs.symbol_, rhs.clock_resets_);
	if (left_tie != right_tie) {
		return left_tie < right_tie;
	}
	// compare clock constraints
	auto lhs_clocks_it = std::begin(lhs.clock_constraints_);
	auto rhs_clocks_it = std::begin(rhs.clock_constraints_);
	while (lhs_clocks_it != std::end(lhs.clock_constraints_)
	       && rhs_clocks_it != std::end(rhs.clock_constraints_)) {
		// compare which clocks are constrained
		if (lhs_clocks_it->first == rhs_clocks_it->first) {
			// compare the constraints on a single clock, if clocks are equal
			if (lhs_clocks_it->second == rhs_clocks_it->second) {
				++lhs_clocks_it;
				++rhs_clocks_it;
				continue;
			} else {
				return lhs_clocks_it->second < rhs_clocks_it->second;
			}
		} else {
			return lhs_clocks_it->first < rhs_clocks_it->first;
		}
		++lhs_clocks_it;
		++rhs_clocks_it;
	}
	if (lhs_clocks_it == std::end(lhs.clock_constraints_)
	    && rhs_clocks_it != std::end(rhs.clock_constraints_)) {
		return true;
	} else if (lhs_clocks_it != std::end(lhs.clock_constraints_)
	           && rhs_clocks_it == std::end(rhs.clock_constraints_)) {
		return false;
	}
	// both are equal
	return false;
}

template <typename LocationT, typename AP>
bool
operator>(const Transition<LocationT, AP> &lhs, const Transition<LocationT, AP> &rhs)
{
	return rhs < lhs;
}

template <typename LocationT, typename AP>
Path<LocationT, AP>::Path(LocationT initial_location, std::set<std::string> clocks)
: current_location_(initial_location), tick_(0)
{
	for (const auto &clock : clocks) {
		clock_valuations_.emplace(std::make_pair(clock, Clock()));
	}
}

template <typename LocationT, typename AP>
void
TimedAutomaton<LocationT, AP>::add_transition(const Transition &transition)
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

template <typename LocationT, typename AP>
std::set<Configuration<LocationT>>
TimedAutomaton<LocationT, AP>::make_symbol_step(const Configuration<LocationT> &configuration,
                                                const AP &                      symbol) const
{
	std::set<Configuration<LocationT>> res;
	// TODO This may cause an issue if the transitions are not sorted as expected, because
	// equal_range returns a sub-sequence
	auto [first, last] = transitions_.equal_range(configuration.location);
	while (first != last) {
		auto trans = std::find_if(first, last, [&](const auto &trans) {
			return trans.second.is_enabled(symbol, configuration.clock_valuations);
		});
		if (trans == last) {
			return res;
		}
		first                         = std::next(trans);
		ClockSetValuation next_clocks = configuration.clock_valuations;
		for (const auto &name : trans->second.clock_resets_) {
			next_clocks[name].reset();
		}
		res.insert(Configuration<LocationT>{trans->second.target_, next_clocks});
	}
	return res;
}

template <typename LocationT, typename AP>
std::set<Path<LocationT, AP>>
TimedAutomaton<LocationT, AP>::make_transition(Path<LocationT, AP> path,
                                               const AP &          symbol,
                                               const Time &        time) const
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
		path.sequence_.emplace_back(symbol, time, target_configuration.location);
		new_path.current_location_ = target_configuration.location;
		new_path.clock_valuations_ = target_configuration.clock_valuations;
		paths.insert(new_path);
	}
	return paths;
}

template <typename LocationT, typename AP>
bool
TimedAutomaton<LocationT, AP>::accepts_word(const TimedWord &word) const
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

template <typename LocationT, typename AP>
std::vector<Transition<LocationT, AP>>
TimedAutomaton<LocationT, AP>::get_enabled_transitions(
  const Configuration<LocationT> &configuration)
{
	std::vector<Transition> res;
	for (const auto &symbol : alphabet_) {
		for (const auto &[source, transition] : transitions_) {
			if (source == configuration.location
			    && transition.is_enabled(symbol, configuration.clock_valuations)) {
				res.push_back(transition);
			}
		}
	}
	return res;
}

template <typename LocationT, typename AP>
Endpoint
TimedAutomaton<LocationT, AP>::get_largest_constant() const
{
	Endpoint res{0};
	for (const auto &[symbol, transition] : transitions_) {
		for (const auto &[symbol, constraint] : transition.get_guards()) {
			const auto candidate =
			  std::visit([](const auto &c) { return c.get_comparand(); }, constraint);
			if (candidate > res) {
				res = candidate;
			}
		}
	}
	return res;
}

template <typename LocationT, typename AP>
Configuration<LocationT>
TimedAutomaton<LocationT, AP>::get_initial_configuration() const
{
	ClockSetValuation clock_valuations;
	for (const auto &clock_name : clocks_) {
		clock_valuations[clock_name] = 0;
	}
	return {initial_location_, clock_valuations};
}

template <typename LocationT, typename AP>
[[nodiscard]] bool
TimedAutomaton<LocationT, AP>::is_accepting_configuration(
  const Configuration<LocationT> &configuration) const
{
	return (final_locations_.find(configuration.location) != final_locations_.end());
}

} // namespace tacos::automata::ta
