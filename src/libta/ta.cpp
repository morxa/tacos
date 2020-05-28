/***************************************************************************
 *  ta.cpp - Core functionality for timed automata
 *
 *  Created: Tue 26 May 2020 13:44:12 CEST 13:44
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

#include <libta/ta.h>

namespace automata {
namespace ta {

TimedAutomaton::TimedAutomaton(const Location &          initial_state,
                               const std::set<Location> &final_states)
: states_{initial_state}, initial_state_(initial_state), final_states_(final_states)
{
	add_states(final_states_);
}

void
TimedAutomaton::add_state(const Location &state)
{
	states_.insert(state);
}

void
TimedAutomaton::add_clock(const std::string &name)
{
	clocks_.insert(name);
}

void
TimedAutomaton::add_states(const std::set<std::string> &states)
{
	for (const auto &state : states) {
		add_state(state);
	}
}

void
TimedAutomaton::add_transition(const Transition &transition)
{
	if (!states_.count(transition.source_)) {
		throw InvalidLocationException(transition.source_);
	}
	if (!states_.count(transition.target_)) {
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

Transition::Transition(const Location &                                   source,
                       const Symbol &                                     symbol,
                       const Location &                                   target,
                       const std::multimap<std::string, ClockConstraint> &clock_constraints,
                       const std::set<std::string> &                      clock_resets)
: source_(source),
  target_(target),
  symbol_(symbol),
  clock_constraints_(clock_constraints),
  clock_resets_(clock_resets)
{
}

std::set<Path>
TimedAutomaton::make_transition(Path path, const Symbol &symbol, const Time &time) const
{
	if (path.tick_ > time) {
		return {};
	}
	for (auto &[name, clock] : path.clock_valuations_) {
		clock.tick(time - path.tick_);
	}
	path.tick_         = time;
	auto [first, last] = transitions_.equal_range(path.current_state_);
	std::set<Path> paths;
	while (first != last) {
		auto trans = std::find_if(first, last, [&](const auto &trans) {
			return trans.second.is_enabled(symbol, path.clock_valuations_);
		});
		if (trans == last) {
			break;
		}
		first               = std::next(trans);
		path.current_state_ = trans->second.target_;
		path.sequence_.push_back(std::make_tuple(symbol, time, path.current_state_));
		for (const auto &name : trans->second.clock_resets_) {
			path.clock_valuations_[name].reset();
		}
		paths.insert(path);
	}
	return paths;
}

bool
TimedAutomaton::accepts_word(const TimedWord &word) const
{
	std::set<Path> paths{Path{initial_state_, clocks_}};
	for (auto &[symbol, time] : word) {
		std::set<Path> res_paths;
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
		if (final_states_.find(path.current_state_) != final_states_.end()) {
			return true;
		};
	}
	return false;
}

bool
Transition::is_enabled(const Symbol &symbol, const std::map<std::string, Clock> &clock_vals) const
{
	if (symbol != symbol_) {
		return false;
	}
	return std::all_of(std::begin(clock_constraints_),
	                   std::end(clock_constraints_),
	                   [&clock_vals](const auto &constraint) {
		                   return std::visit(
		                     [&](auto &&c) {
			                     return c.is_satisfied(clock_vals.at(constraint.first).get_valuation());
		                     },
		                     constraint.second);
	                   });
}

Path::Path(std::string initial_state, std::set<std::string> clocks)
: current_state_(initial_state), tick_(0)
{
	for (const auto &clock : clocks) {
		clock_valuations_.emplace(std::make_pair(clock, Clock()));
	}
}

bool
operator<(const Path &p1, const Path &p2)
{
	return p1.sequence_ < p2.sequence_;
}

} // namespace ta
} // namespace automata
