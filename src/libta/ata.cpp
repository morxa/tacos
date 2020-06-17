/***************************************************************************
 *  ata.cpp - Alternating Timed Automata
 *
 *  Created: Fri 05 Jun 2020 11:54:51 CEST 11:54
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

#include <libta/ata.h>
#include <libta/automata.h>

#include <cassert>
#include <iterator>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/cartesian_product.hpp>
#include <variant>

namespace automata::ata {

Transition::Transition(const Location &         source,
                       const Symbol &           symbol,
                       std::unique_ptr<Formula> formula)
: source_(source), symbol_(symbol), formula_(std::move(formula))
{
}

AlternatingTimedAutomaton::AlternatingTimedAutomaton(const std::set<Symbol> &  alphabet,
                                                     const Location &          initial_location,
                                                     const std::set<Location> &final_locations,
                                                     std::set<Transition>      transitions)
: alphabet_(alphabet),
  initial_location_(initial_location),
  final_locations_(final_locations),
  transitions_(std::move(transitions))
{
}
std::vector<Run>
AlternatingTimedAutomaton::make_symbol_transition(const std::vector<Run> &runs,
                                                  const Symbol &          symbol) const
{
	std::vector<Run> res;
	for (const auto &run : runs) {
		if (!run.empty() && std::holds_alternative<Symbol>(run.back().first)) {
			throw WrongTransitionTypeException(
			  "Cannot do two subsequent symbol transitions, transitions must be "
			  "alternating between symbol and time");
		}
		std::set<State> start_states;
		if (run.empty()) {
			start_states = {State(initial_location_, 0)};
		} else {
			start_states = run.back().second;
		}
		// A vector of a set of target configurations that are reached when following a transition
		// One entry for each start state
		std::vector<std::set<Configuration>> models;
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
		assert(!models.empty());
		if (models.empty()) {
			return {};
		}
		// The resulting configurations after computing the cartesian product of all target
		// configurations of each state
		std::set<Configuration> configurations;
		// Populate the configurations by splitting the first configuration in all minimal models
		// { { m1, m2, m3 } } -> { { m1 }, { m2 }, { m3 } }
		ranges::for_each(models[0],
		                 [&](const auto &state_model) { configurations.insert({state_model}); });
		// Add models from the other configurations
		std::for_each(std::next(models.begin()), models.end(), [&](const auto &state_models) {
			std::set<Configuration> expanded_configurations;
			ranges::for_each(state_models, [&](const auto &state_model) {
				ranges::for_each(configurations, [&](const auto &configuration) {
					auto expanded_configuration = configuration;
					expanded_configuration.insert(state_model.begin(), state_model.end());
					expanded_configurations.insert(expanded_configuration);
				});
			});
			configurations = expanded_configurations;
		});
		ranges::for_each(configurations, [&](const auto &c) {
			auto expanded_run = run;
			expanded_run.push_back(std::make_pair(std::variant<Symbol, Time>(symbol), c));
			res.push_back(expanded_run);
		});
	}
	return res;
}

std::vector<Run>
AlternatingTimedAutomaton::make_time_transition(const std::vector<Run> &runs,
                                                const Time &            time) const
{
	if (time < 0) {
		throw NegativeTimeDeltaException(time);
	}
	std::vector<Run> res;
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
		std::set<State> res_states;
		std::transform(run.back().second.cbegin(),
		               run.back().second.cend(),
		               std::inserter(res_states, res_states.begin()),
		               [&time](const auto &s) { return State(s.first, s.second + time); });
		run.push_back(std::make_pair(time, res_states));
		res.push_back(run);
	}
	return res;
}

bool
AlternatingTimedAutomaton::accepts_word(const TimedWord &word) const
{
	if (word.size() == 0) {
		return false;
	}
	std::vector<Run> runs = {{}};
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

std::set<std::set<State>>
AlternatingTimedAutomaton::get_minimal_models(Formula *formula, ClockValuation v) const
{
	return formula->get_minimal_models(v);
}

bool
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

} // namespace automata::ata
