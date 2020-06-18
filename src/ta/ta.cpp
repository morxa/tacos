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

#include <ta/ta.h>

namespace automata {
namespace ta {

TimedAutomaton::TimedAutomaton(const Location &          initial_location,
                               const std::set<Location> &final_locations)
: locations_{initial_location},
  initial_location_(initial_location),
  final_locations_(final_locations)
{
	add_locations(final_locations_);
}

void
TimedAutomaton::add_location(const Location &location)
{
	locations_.insert(location);
}

void
TimedAutomaton::add_clock(const std::string &name)
{
	clocks_.insert(name);
}

void
TimedAutomaton::add_locations(const std::set<std::string> &locations)
{
	for (const auto &location : locations) {
		add_location(location);
	}
}

void
TimedAutomaton::add_transition(const Transition &transition)
{
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
	auto [first, last] = transitions_.equal_range(path.current_location_);
	std::set<Path> paths;
	while (first != last) {
		auto trans = std::find_if(first, last, [&](const auto &trans) {
			return trans.second.is_enabled(symbol, path.clock_valuations_);
		});
		if (trans == last) {
			break;
		}
		first                  = std::next(trans);
		path.current_location_ = trans->second.target_;
		path.sequence_.push_back(std::make_tuple(symbol, time, path.current_location_));
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
	std::set<Path> paths{Path{initial_location_, clocks_}};
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
		if (final_locations_.find(path.current_location_) != final_locations_.end()) {
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
		                   return is_satisfied(constraint.second,
		                                       clock_vals.at(constraint.first).get_valuation());
	                   });
}

Path::Path(std::string initial_location, std::set<std::string> clocks)
: current_location_(initial_location), tick_(0)
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
