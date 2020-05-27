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
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace ta {

using State     = std::string;
using Symbol    = std::string;
using Time      = double;
using TimedWord = std::vector<std::pair<Symbol, Time>>;

class Clock
{
public:
	constexpr Clock() : valuation_{0}
	{
	}

	constexpr void
	tick(const Time &diff)
	{
		valuation_ += diff;
	};

	constexpr Time
	get_valuation() const
	{
		return valuation_;
	}
	constexpr void
	reset()
	{
		valuation_ = 0;
	}

private:
	Time valuation_;
};

template <class Comp>
class AtomicClockConstraintT
{
public:
	AtomicClockConstraintT(const Time &comparand) : comparand_(comparand)
	{
	}
	constexpr bool
	is_satisfied(const Time &valuation) const
	{
		return Comp()(valuation, comparand_);
	}

private:
	const Time comparand_;
};

using ClockConstraint = std::variant<AtomicClockConstraintT<std::less<Time>>,
                                     AtomicClockConstraintT<std::less_equal<Time>>,
                                     AtomicClockConstraintT<std::equal_to<Time>>,
                                     AtomicClockConstraintT<std::greater_equal<Time>>,
                                     AtomicClockConstraintT<std::greater<Time>>>;

class Transition
{
public:
	friend class TimedAutomaton;
	Transition(const State &                                      source,
	           const Symbol &                                     symbol,
	           const State &                                      target,
	           const std::multimap<std::string, ClockConstraint> &clock_constraints = {},
	           const std::set<std::string> &                      clock_resets      = {});
	bool is_enabled(const Symbol &symbol, const std::map<std::string, Clock> &clock_vals) const;

private:
	State                                       source_;
	State                                       target_;
	Symbol                                      symbol_;
	std::multimap<std::string, ClockConstraint> clock_constraints_;
	std::set<std::string>                       clock_resets_;
};

class TimedAutomaton
{
public:
	TimedAutomaton() = delete;
	TimedAutomaton(const State &initial_state, const std::set<State> &final_states);
	TimedAutomaton(const std::set<std::string> &states);
	void add_state(const State &state);
	void add_clock(const std::string &name);
	void add_states(const std::set<std::string> &states);
	void set_initial_state(const State &state);
	void add_transition(const Transition &transition);
	bool make_transition(const Symbol &symbol, const Time &time);
	bool accepts_word(const TimedWord &);
	void reset();

private:
	std::set<State>                  states_;
	const State                      initial_state_;
	const std::set<State>            final_states_;
	State                            current_state_;
	Time                             tick_;
	std::map<std::string, Clock>     clocks_;
	std::multimap<State, Transition> transitions_;
};

} // namespace ta
