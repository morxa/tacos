/***************************************************************************
 *  ata.hpp - Alternating Timed Automata
 *
 *  Created:   Tue  9 Feb 17:49:19 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include "ata.h"

namespace tacos::automata::ata {

template <typename LocationT, typename SymbolT>
std::ostream &
operator<<(std::ostream &os, const tacos::automata::ata::Transition<LocationT, SymbolT> &transition)
{
	os << transition.source_ << u8" → " << transition.symbol_ << u8" → " << *transition.formula_;
	return os;
}

template <typename LocationT, typename SymbolT>
std::ostream &
operator<<(std::ostream                                                              &os,
           const tacos::automata::ata::AlternatingTimedAutomaton<LocationT, SymbolT> &ata)
{
	os << "Alphabet: {";
	{
		bool first = true;
		for (const auto &symbol : ata.alphabet_) {
			if (!first) {
				os << ", ";
			} else {
				first = false;
			}
			os << symbol;
		}
	}
	os << "}";
	os << ", initial location: " << ata.initial_location_;
	os << ", final locations: {";
	{
		bool first = true;
		for (const auto &location : ata.final_locations_) {
			if (!first) {
				os << ", ";
			} else {
				first = false;
			}
			os << location;
		}
	}
	os << "}";
	if (ata.sink_location_.has_value()) {
		os << ", sink location: " << ata.sink_location_.value();
	} else {
		os << ", no sink location";
	}
	os << ", transitions:";
	for (const auto &transition : ata.transitions_) {
		os << '\n' << "  " << transition;
	}

	return os;
}

template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const tacos::automata::ata::Configuration<LocationT> &configuration)
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

template <typename LocationT, typename SymbolT>
std::ostream &
operator<<(std::ostream &os, const tacos::automata::ata::Run<LocationT, SymbolT> &run)
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

template <typename LocationT, typename SymbolT>
bool
operator<(const Transition<LocationT, SymbolT> &first, const Transition<LocationT, SymbolT> &second)
{
	if (first.source_ != second.source_) {
		return first.source_ < second.source_;
	} else if (first.symbol_ != second.symbol_) {
		return first.symbol_ < second.symbol_;
	} else {
		return first.formula_ < second.formula_;
	}
}

template <typename LocationT, typename SymbolT>
Transition<LocationT, SymbolT>::Transition(const LocationT                    &source,
                                           const SymbolT                      &symbol,
                                           std::unique_ptr<Formula<LocationT>> formula)
: source_(source), symbol_(symbol), formula_(std::move(formula))
{
}

template <typename LocationT, typename SymbolT>
AlternatingTimedAutomaton<LocationT, SymbolT>::AlternatingTimedAutomaton(
  const std::set<SymbolT>                 &alphabet,
  const LocationT                         &initial_location,
  const std::set<LocationT>               &final_locations,
  std::set<Transition<LocationT, SymbolT>> transitions,
  std::optional<LocationT>                 sink_location)
: alphabet_(alphabet),
  initial_location_(initial_location),
  final_locations_(final_locations),
  transitions_(std::move(transitions)),
  sink_location_(sink_location)
{
	if (sink_location_) {
		if (initial_location_ == *sink_location_) {
			throw std::invalid_argument("The initial location must not be the sink location");
		}
		if (final_locations_.find(*sink_location_) != std::end(final_locations_)) {
			throw std::invalid_argument("The sink location must not be a final location");
		}
		for (const auto &transition : transitions_) {
			if (transition.source_ == *sink_location_) {
				throw std::invalid_argument("A transition may not contain the sink location as source");
			}
		}
	}
}

template <typename LocationT, typename SymbolT>
[[nodiscard]] Configuration<LocationT>
AlternatingTimedAutomaton<LocationT, SymbolT>::get_initial_configuration() const
{
	return {State<LocationT>{initial_location_, 0}};
}

template <typename LocationT, typename SymbolT>
std::set<Configuration<LocationT>>
AlternatingTimedAutomaton<LocationT, SymbolT>::make_symbol_step(
  const Configuration<LocationT> &start_states,
  const SymbolT                  &symbol) const
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
			return t.source_ == state.location && t.symbol_ == symbol;
		});
		if (t == transitions_.end()) {
			continue;
		}
		const auto new_states = get_minimal_models(t->formula_.get(), state.clock_valuation);
		models.push_back(new_states);
	}
	// We were not able to make any transition.
	if (models.empty() || std::any_of(std::begin(models), std::end(models), [](const auto &model) {
		    return model.empty();
	    })) {
		// We have a sink location, the next configuration is the singleton {(sink, 0)}.
		if (sink_location_.has_value()) {
			return {{State<LocationT>{sink_location_.value(), 0}}};
		} else {
			// No sink location, return the empty set. The ATA is incomplete.
			return {};
		}
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
	// If we get here and the configurations are empty, something went wrong. If there is a transition
	// without model, this should have been caught earlier.
	assert(!configurations.empty());
	return configurations;
}

template <typename LocationT, typename SymbolT>
std::vector<Run<LocationT, SymbolT>>
AlternatingTimedAutomaton<LocationT, SymbolT>::make_symbol_transition(
  const std::vector<Run<LocationT, SymbolT>> &runs,
  const SymbolT                              &symbol) const
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
			start_states = get_initial_configuration();
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

template <typename LocationT, typename SymbolT>
Configuration<LocationT>
AlternatingTimedAutomaton<LocationT, SymbolT>::make_time_step(const Configuration<LocationT> &start,
                                                              const Time &time) const
{
	if (time < 0) {
		throw NegativeTimeDeltaException(time);
	}
	Configuration<LocationT> res;
	std::transform(start.cbegin(),
	               start.cend(),
	               std::inserter(res, res.begin()),
	               [&time](const auto &s) {
		               return State<LocationT>{s.location, s.clock_valuation + time};
	               });
	return res;
}

template <typename LocationT, typename SymbolT>
std::vector<Run<LocationT, SymbolT>>
AlternatingTimedAutomaton<LocationT, SymbolT>::make_time_transition(
  const std::vector<Run<LocationT, SymbolT>> &runs,
  const Time                                 &time) const
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
		run.emplace_back(time, make_time_step(run.back().second, time));
		res.push_back(run);
	}
	return res;
}

template <typename LocationT, typename SymbolT>
[[nodiscard]] bool
AlternatingTimedAutomaton<LocationT, SymbolT>::is_accepting_configuration(
  const Configuration<LocationT> &configuration) const
{
	return std::all_of(configuration.begin(), configuration.end(), [this](auto const &state) {
		return final_locations_.count(state.location) > 0;
	});
}

template <typename LocationT, typename SymbolT>
[[nodiscard]] bool
AlternatingTimedAutomaton<LocationT, SymbolT>::accepts_word(const TimedATAWord<SymbolT> &word) const
{
	if (word.size() == 0) {
		return false;
	}
	if (word[0].second != 0) {
		throw InvalidTimedWordException("Invalid time initialization " + std::to_string(word[0].second)
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
		return is_accepting_configuration(final_configuration);
	});
}

template <typename LocationT, typename SymbolT>
std::set<std::set<State<LocationT>>>
AlternatingTimedAutomaton<LocationT, SymbolT>::get_minimal_models(Formula<LocationT> *formula,
                                                                  ClockValuation      v) const
{
	return formula->get_minimal_models(v);
}

} // namespace tacos::automata::ata
