/***************************************************************************
 *  ta_product.h - Compute the product automaton of timed automata
 *
 *  Created:   Mon  1 Mar 12:49:54 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


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

namespace tacos::automata::ta {

template <typename ActionT>
std::map<ActionT, std::vector<std::size_t>>
collect_synchronizing_alphabets(const std::set<ActionT> &             synchronized_actions,
                                const std::vector<std::set<ActionT>> &alphabets)
{
	std::map<ActionT, std::vector<std::size_t>> res;
	for (const auto &symbol : synchronized_actions) {
		res[symbol] = {};
		for (std::size_t i = 0; i < alphabets.size(); ++i) {
			if (alphabets[i].find(symbol) != std::end(alphabets[i])) {
				res[symbol].push_back(i);
			}
		}
	}
	return res;
}

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
	// datastructures required for synchronization
	std::vector<std::set<ActionT>> alphabets;
	// build alphabet, initial location and clocks of the product
	std::for_each(std::begin(automata), std::end(automata), [&](const auto &ta) {
		alphabets.push_back(ta.get_alphabet());
		product_alphabet.insert(std::begin(ta.get_alphabet()), std::end(ta.get_alphabet()));
		product_initial_location->push_back(ta.get_initial_location().get());
		product_clocks.insert(std::begin(ta.get_clocks()), std::end(ta.get_clocks()));
	});
	// collect which automata synchronize on which actions
	std::map<ActionT, std::vector<std::size_t>> synchronizing_actions =
	  collect_synchronizing_alphabets(synchronized_actions, alphabets);
	// collects synchronizing transitions
	std::map<ActionT, std::set<Transition<std::vector<LocationT>, ActionT>>> synchronized_transitions;

	for (const auto &symbol : product_alphabet) {
		bool symbol_synchronizes =
		  (synchronizing_actions.find(symbol) != std::end(synchronizing_actions));
		for (const auto &[ta_i, ta] : ranges::views::enumerate(automata)) {
			// before finalizing the construction for one automaton, we collect all candidates for
			// synchronizing jumps to avoid expensive lookup.
			std::map<ActionT, std::set<Transition<std::vector<LocationT>, ActionT>>>
			  new_synchronizing_jumps;
			for (const auto &[source_location, transition] : ta.get_transitions()) {
				// Only consider transitions where the symbol matches. Note that if the automaton does not
				// synchronize with this symbol, this check will always fail since implicitly an automaton
				// can only synchronize with a synchronizing symbol if it has at least one transition with
				// this symbol. This information is relevant to establish, that the automaton is able to
				// synchronize with this symbol in the second case (else-branch below).
				if (transition.symbol_ == symbol) {
					// Handling of non-synchronized transitions: insert transitions for all product locations
					// where source and target of the local transition match.
					if (!symbol_synchronizes) {
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
						// We know the automaton *must* synchronize with the symbol
						assert(std::find(std::begin(synchronizing_actions[symbol]),
						                 std::end(synchronizing_actions[symbol]),
						                 ta_i)
						       != std::end(synchronizing_actions[symbol]));
						// if this is the first synchronized transition with this symbol, initialize set of
						// potentially synchronizing jumps.
						if (synchronized_transitions[symbol].empty()) {
							for (const auto &product_source_location : product_locations) {
								if (product_source_location.get()[ta_i] == transition.source_.get()) {
									auto product_target_location        = product_source_location;
									product_target_location.get()[ta_i] = transition.target_.get();
									new_synchronizing_jumps[symbol].emplace(product_source_location,
									                                        transition.symbol_,
									                                        product_target_location,
									                                        transition.clock_constraints_,
									                                        transition.clock_resets_);
								}
							}
						} else {
							// there are already transition-candidates for synchronization, we need to create
							// candidates for each transition in the current automaton which also synchronizes.
							for (const auto &sync_transition : synchronized_transitions[symbol]) {
								auto product_source_location        = sync_transition.source_;
								product_source_location.get()[ta_i] = transition.source_.get();
								auto product_target_location        = sync_transition.target_;
								product_target_location.get()[ta_i] = transition.target_.get();
								auto constraints                    = sync_transition.clock_constraints_;
								constraints.insert(std::begin(transition.clock_constraints_),
								                   std::end(transition.clock_constraints_));
								auto resets = sync_transition.clock_resets_;
								resets.insert(std::begin(transition.clock_resets_),
								              std::end(transition.clock_resets_));
								new_synchronizing_jumps[symbol].emplace(product_source_location,
								                                        transition.symbol_,
								                                        product_target_location,
								                                        constraints,
								                                        resets);
							}
						}
					}
				}
			}
			// update transitions
			for (const auto &[symbol, transitions] : new_synchronizing_jumps) {
				synchronized_transitions[symbol] = std::move(transitions);
			}
		}
	}
	// update transitions
	for (const auto &[symbol, transitions] : synchronized_transitions) {
		std::copy(std::begin(transitions),
		          std::end(transitions),
		          std::back_inserter(product_transitions));
	}

	return TimedAutomaton<std::vector<LocationT>, ActionT>{product_locations,
	                                                       product_alphabet,
	                                                       product_initial_location,
	                                                       product_final_locations,
	                                                       product_clocks,
	                                                       product_transitions};
}

} // namespace tacos::automata::ta
