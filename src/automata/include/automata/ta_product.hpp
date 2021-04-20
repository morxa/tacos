/***************************************************************************
 *  ta_product.h - Compute the product automaton of timed automata
 *
 *  Created:   Mon  1 Mar 12:49:54 CET 2021
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

#include "automata/ta.h"
#include "ta_product.h"

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#include <ctime>
#include <iterator>
#include <range/v3/view/enumerate.hpp>
#include <stdexcept>
#include <tuple>

namespace automata::ta {

template <typename LocationT, typename ActionT>
TimedAutomaton<std::vector<LocationT>, ActionT>
get_product(const std::vector<TimedAutomaton<LocationT, ActionT>> &automata,
            const std::set<ActionT> &                              synchronized_actions)
{
	if (automata.empty()) {
		throw std::invalid_argument("Cannot compute product of zero automata");
	}
	for (auto it1 = std::begin(automata); it1 != std::prev(std::end(automata)); ++it1) {
		for (auto it2 = std::next(it1); it2 != std::end(automata); ++it2) {
			std::set<std::string> common_clocks;
			std::set_intersection(begin(it1->get_clocks()),
			                      end(it1->get_clocks()),
			                      begin(it2->get_clocks()),
			                      end(it2->get_clocks()),
			                      std::inserter(common_clocks, end(common_clocks)));

			if (!common_clocks.empty()) {
				throw std::invalid_argument(
				  fmt::format("Cannot construct product automaton for two automata with non-disjoint "
				              "clocks, common clocks: {}",
				              fmt::join(common_clocks, ", ")));
			}
		}
	}
	using ProductLocation = Location<std::vector<LocationT>>;
	std::set<ActionT>         actions;
	std::set<ProductLocation> product_locations;
	// Initialize the product locations with the locations from the first TA, by creating a set of
	// Location<std::vector<LocationT>> from the set of Location<LocationT>.
	std::transform(std::begin(std::begin(automata)->get_locations()),
	               std::end(std::begin(automata)->get_locations()),
	               std::inserter(product_locations, end(product_locations)),
	               [](const auto &location) { return ProductLocation{{location.get()}}; });
	// Add the locations from the remaining TAs.
	std::for_each(std::next(std::begin(automata)),
	              std::end(automata),
	              [&product_locations](const auto &ta) {
		              std::set<ProductLocation> augmented_product_locations;
		              for (const auto &product_location : product_locations) {
			              for (const auto &ta_location : ta.get_locations()) {
				              auto new_product_location = product_location;
				              new_product_location->push_back(ta_location);
				              augmented_product_locations.insert(new_product_location);
			              }
		              }
		              product_locations = augmented_product_locations;
	              });
	std::set<ProductLocation> product_final_locations;
	// The same as above, but for the final locations.
	std::transform(std::begin(std::begin(automata)->get_final_locations()),
	               std::end(std::begin(automata)->get_final_locations()),
	               std::inserter(product_final_locations, end(product_final_locations)),
	               [](const auto &location) { return ProductLocation{{location.get()}}; });
	std::for_each(std::next(std::begin(automata)),
	              std::end(automata),
	              [&product_final_locations](const auto &ta) {
		              std::set<ProductLocation> augmented_product_final_locations;
		              for (const auto &product_location : product_final_locations) {
			              for (const auto &ta_location : ta.get_final_locations()) {
				              auto new_product_location = product_location;
				              new_product_location->push_back(ta_location);
				              augmented_product_final_locations.insert(new_product_location);
			              }
		              }
		              product_final_locations = augmented_product_final_locations;
	              });

	// Assert that each product location consists of one location from each TA.
	assert(std::all_of(begin(product_locations),
	                   end(product_locations),
	                   [&automata](const auto &location) {
		                   return location.get().size() == automata.size();
	                   }));

	std::set<ActionT>                                        product_alphabet;
	ProductLocation                                          product_initial_location;
	std::set<std::string>                                    product_clocks;
	std::vector<Transition<std::vector<LocationT>, ActionT>> product_transitions;
	std::multimap<ActionT, Transition<std::vector<LocationT>, ActionT>>
	  synchronized_transition_candidates;
	for (const auto &[ta_i, ta] : ranges::views::enumerate(automata)) {
		product_alphabet.insert(std::begin(ta.get_alphabet()), std::end(ta.get_alphabet()));
		product_initial_location->push_back(ta.get_initial_location().get());
		product_clocks.insert(std::begin(ta.get_clocks()), std::end(ta.get_clocks()));
		for (const auto &[source_location, transition] : ta.get_transitions()) {
			// handling of non-synchronized transitions
			if (std::find(std::begin(synchronized_actions),
			              std::end(synchronized_actions),
			              transition.symbol_)
			    == std::end(synchronized_actions)) {
				for (const auto &product_source_location : product_locations) {
					if (product_source_location.get()[ta_i] == transition.source_.get()) {
						auto product_target_location        = product_source_location;
						product_target_location.get()[ta_i] = transition.target_.get();
						product_transitions.emplace_back(product_source_location,
						                                 transition.symbol_,
						                                 product_target_location,
						                                 transition.clock_constraints_,
						                                 transition.clock_resets_);
					}
				}
			} else {
				// Synchronized jumps will be constructend iteratively. We build candidates iteratively by
				// creating transitions for all combinations of source and target location which occur in
				// the original automata. For a given automaton with a synchronizing jump on symbol "a" we
				// create all combinations of jumps also labelled with "a" from all other automata.
				auto candidates = synchronized_transition_candidates.equal_range(transition.symbol_);
				if (candidates.first == candidates.second) {
					// initialize candidate for synchronized jump
					Transition<std::vector<LocationT>, ActionT> new_transition =
					  Transition<std::vector<LocationT>, ActionT>(ProductLocation({transition.source_.get()}),
					                                              transition.symbol_,
					                                              ProductLocation({transition.target_.get()}),
					                                              transition.clock_constraints_,
					                                              transition.clock_resets_);
					synchronized_transition_candidates.insert(
					  std::make_pair(transition.symbol_, new_transition));
				} else {
					// Augment existing candidates. If this is the first candidate for this automaton, the
					// vector of source and target locations of the existing candidates will be of length i-1
					// (i is the automaton index), otherwise the length will be i. In the later case we need
					// to duplicate all candidates in order to be able to create all combinations of
					// synchronizing jumps.
					if (candidates.first->second.source_.get().size() == ta_i) {
						// first case: augment existing candidates by this new candidate-component.
						for (auto it = candidates.first; it != candidates.second; ++it) {
							auto &[symbol, candidate] = *it;
							candidate.source_.get().push_back(transition.source_);
							candidate.target_.get().push_back(transition.source_);
							candidate.clock_constraints_.insert(std::begin(transition.clock_constraints_),
							                                    std::end(transition.clock_constraints_));
							candidate.clock_resets_.insert(std::begin(transition.clock_resets_),
							                               std::end(transition.clock_resets_));
						}
					} else {
						// second case: there was already a synchronizing jump for this automaton, create
						// duplicates.
						std::vector<Transition<std::vector<LocationT>, ActionT>> new_combinations;
						for (auto it = candidates.first; it != candidates.second; ++it) {
							auto &[symbol, candidate] = *it;
							// update source and target
							std::vector<LocationT> sources{std::begin(candidate.source_.get()),
							                               std::prev(std::end(candidate.source_.get()), 2)};
							std::vector<LocationT> targets{std::begin(candidate.target_.get()),
							                               std::prev(std::end(candidate.target_.get()), 2)};
							sources.push_back(transition.source_);
							targets.push_back(transition.target_);
							// remove clock constraints which affect clocks of the current automaton to allow
							// replacement with new constraints of the duplicate
							auto guards = candidate.clock_constraints_;
							std::for_each(std::begin(ta.get_clocks()),
							              std::end(ta.get_clocks()),
							              [&guards](const auto &clock) { guards.erase(clock); });
							guards.insert(std::begin(transition.clock_constraints_),
							              std::end(transition.clock_constraints_));
							// remove clock resets which affect clocks of the current automaton to allow
							// replacement with new resets of the duplicate
							auto resets = candidate.clock_resets_;
							std::for_each(std::begin(ta.get_clocks()),
							              std::end(ta.get_clocks()),
							              [&resets](const auto &clock) { resets.erase(clock); });
							resets.insert(std::begin(transition.clock_resets_),
							              std::end(transition.clock_resets_));
							// create duplicate
							new_combinations.emplace_back(
							  ProductLocation(sources), symbol, ProductLocation(targets), guards, resets);
						}
						// insert duplicates into candidates list
						std::transform(std::begin(new_combinations),
						               std::end(new_combinations),
						               std::inserter(synchronized_transition_candidates,
						                             std::end(synchronized_transition_candidates)),
						               [](const auto &transition) {
							               return std::make_pair(transition.symbol_, transition);
						               });
					}
				}
			}
		}
	}

	// take only those synchronized actions on which *all* automata agree
	for (const auto &[symbol, transition] : synchronized_transition_candidates) {
		if (transition.source_.get().size() == automata.size()) {
			product_transitions.emplace_back(transition);
		}
	}

	return TimedAutomaton<std::vector<LocationT>, ActionT>{product_locations,
	                                                       product_alphabet,
	                                                       product_initial_location,
	                                                       product_final_locations,
	                                                       product_clocks,
	                                                       product_transitions};
}

} // namespace automata::ta
