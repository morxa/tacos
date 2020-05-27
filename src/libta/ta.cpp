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

#include <algorithm>
#include <cassert>

namespace ta {

TimedAutomaton::TimedAutomaton(const State &initial_state, const std::set<State> &final_states)
: states_{initial_state},
  initial_state_(initial_state),
  final_states_(final_states),
  current_state_{initial_state_},
  tick_{0}
{
	add_states(final_states_);
}

void
TimedAutomaton::add_state(const State &state)
{
	states_.insert(state);
}

void
TimedAutomaton::add_clock(const std::string &name)
{
	clocks_[name] = Clock();
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
	transitions_.insert({transition.source_, transition});
}

Transition::Transition(const State &                                      source,
                       const Symbol &                                     symbol,
                       const State &                                      target,
                       const std::multimap<std::string, ClockConstraint> &clock_constraints,
                       const std::set<std::string> &                      clock_resets)
: source_(source),
  target_(target),
  symbol_(symbol),
  clock_constraints_(clock_constraints),
  clock_resets_(clock_resets)
{
}

bool
TimedAutomaton::make_transition(const Symbol &symbol, const Time &time)
{
	if (tick_ > time) {
		return false;
	}
	for (auto &clock : clocks_) {
		clock.second.tick(time - tick_);
	}
	tick_      = time;
	auto trans = transitions_.find(current_state_);
	while (trans != transitions_.end() && trans->first == current_state_) {
		if (trans->second.is_enabled(symbol, clocks_)) {
			current_state_ = trans->second.target_;
			for (auto &reset : trans->second.clock_resets_) {
				clocks_[reset].reset();
			}
			return true;
		}
		++trans;
	}
	return false;
}

bool
TimedAutomaton::accepts_word(const TimedWord &word)
{
	for (auto &[symbol, time] : word) {
		if (!make_transition(symbol, time)) {
			return false;
		}
	}
	return final_states_.find(current_state_) != final_states_.end();
}

void
TimedAutomaton::reset()
{
	tick_          = 0;
	current_state_ = initial_state_;
	for (auto &[id, c] : clocks_) {
		c.reset();
	}
}

bool
Transition::is_enabled(const Symbol &symbol, const std::map<std::string, Clock> &clock_vals)
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

} // namespace ta
