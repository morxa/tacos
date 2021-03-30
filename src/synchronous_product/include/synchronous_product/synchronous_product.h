/***************************************************************************
 *  synchronous_product.h - The synchronous product of a TA and an ATA
 *
 *  Created:   Mon 21 Dec 16:13:49 CET 2020
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
 *  Read the full text in the LICENSE.md file.
 */

#pragma once

#include "automata/ata.h"
#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_regions.h"
#include "canonical_word.h"
#include "mtl/MTLFormula.h"
#include "utilities/numbers.h"

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace synchronous_product {

/** Get the clock valuation for an ABSymbol, which is either a TA state or an ATA state.
 * @param w The symbol to read the time from
 * @return The clock valuation in the given state
 */
template <typename Location, typename ActionType>
ClockValuation
get_time(const ABSymbol<Location, ActionType> &w)
{
	if (std::holds_alternative<TAState<Location>>(w)) {
		return std::get<2>(std::get<TAState<Location>>(w));
	} else {
		return std::get<ATAState<ActionType>>(w).second;
	}
}

/** Get the clock valuation for an ABRegionSymbol, which is either a
 * TARegionState or an ATARegionState.
 * @param w The symbol to read the time from
 * @return The region index in the given state
 */
template <typename Location, typename ActionType>
RegionIndex
get_region_index(const ABRegionSymbol<Location, ActionType> &w)
{
	if (std::holds_alternative<TARegionState<Location>>(w)) {
		return std::get<2>(std::get<TARegionState<Location>>(w));
	} else {
		return std::get<ATARegionState<ActionType>>(w).second;
	}
}

/** Thrown if a canonical word is not valid. */
class InvalidCanonicalWordException : public std::domain_error
{
public:
	/** Construct the exception with a single message
	 * @param message The exact error that occurred
	 */
	explicit InvalidCanonicalWordException(const std::string &message)
	: std::domain_error(""), message_(message)
	{
	}

	/** Construct the exception with the word and a message
	 * @param word The word that is invalid
	 * @param error The error that occurred
	 */
	template <typename Location, typename ActionType>
	explicit InvalidCanonicalWordException(const CanonicalABWord<Location, ActionType> &word,
	                                       const std::string &                          error)
	: std::domain_error("")
	{
		std::stringstream msg;
		msg << "Invalid word: '" << word << "': " << error;
		message_ = msg.str();
	}

	/** Get the exact error message
	 * @return A message describing the error in detail
	 */
	const char *
	what() const noexcept override
	{
		return message_.c_str();
	}

private:
	std::string message_;
};

/** Validate a canonical word.
 * Check a word whether it is a valid canonical word. Throws an exception if this is not the case.
 * @param word The word to check
 * @return true if the word is a valid canonical word
 */
template <typename Location, typename ActionType>
bool
is_valid_canonical_word(const CanonicalABWord<Location, ActionType> &word)
{
	// TODO all ta_symbols should agree on the same location
	// TODO clocks must have unique values (i.e., must not occur multiple times)
	if (word.empty()) {
		throw InvalidCanonicalWordException(word, "word is empty");
	}
	// No configuration should be empty
	if (std::any_of(word.begin(), word.end(), [](const auto &configurations) {
		    return configurations.empty();
	    })) {
		throw InvalidCanonicalWordException(word, "word contains an empty configuration");
	}
	// Each partition either contains only even or only odd region indexes. This is because the word
	// is partitioned by the fractional part and the region index can only be even if the fractional
	// part is 0. If that is the case, there cannot be any configuration with an odd region index in
	// the same partition, as that configuration's fractional part would be > 0.
	std::for_each(word.begin(), word.end(), [&word](const auto &configurations) {
		if (std::any_of(configurations.begin(),
		                configurations.end(),
		                [](const auto &w) { return get_region_index(w) % 2 == 0; })
		    && std::any_of(configurations.begin(), configurations.end(), [](const auto &w) {
			       return get_region_index(w) % 2 == 1;
		       })) {
			throw InvalidCanonicalWordException(word, "both odd and even region indexes");
		}
	});
	// There must be at most one partition with fractional part 0.
	// The only partition that is allowed to have fracitonal part 0 is the 0th
	// partition.
	std::for_each(std::next(word.begin()), word.end(), [&word](const auto &configurations) {
		std::for_each(configurations.begin(), configurations.end(), [&word](const auto &w) {
			if (get_region_index(w) % 2 == 0) {
				throw InvalidCanonicalWordException(word,
				                                    "fractional part 0 in wrong element of partition");
			}
		});
	});
	return true;
}

/// Increment the region indexes in the configurations of the given ABRegionSymbol.
/** This is a helper function to increase the region index so we reach the next region set.
 * @param configurations The set of configurations to increment the region indexes in
 * @param max_region_index The maximal region index that may never be passed
 * @return A copy of the given configurations with incremented region indexes
 */
template <typename Location, typename ActionType>
std::set<ABRegionSymbol<Location, ActionType>>
increment_region_indexes(const std::set<ABRegionSymbol<Location, ActionType>> &configurations,
                         RegionIndex                                           max_region_index)
{
	// Assert that our assumption holds: All region indexes are either odd or even, never mixed.
	assert(
	  std::all_of(std::begin(configurations),
	              std::end(configurations),
	              [](const auto &configuration) { return get_region_index(configuration) % 2 == 0; })
	  || std::all_of(std::begin(configurations),
	                 std::end(configurations),
	                 [](const auto &configuration) {
		                 return get_region_index(configuration) % 2 == 1;
	                 }));
	std::set<ABRegionSymbol<Location, ActionType>> res;
	std::transform(configurations.begin(),
	               configurations.end(),
	               std::inserter(res, res.end()),
	               [max_region_index](auto configuration) {
		               if (std::holds_alternative<TARegionState<Location>>(configuration)) {
			               auto &ta_configuration    = std::get<TARegionState<Location>>(configuration);
			               RegionIndex &region_index = std::get<2>(ta_configuration);
			               // Increment if the region index is less than the max_region_index and if the
			               // region index is even or if we increment all region indexes.
			               if (region_index < max_region_index) {
				               region_index += 1;
			               }
		               } else {
			               auto &ata_configuration = std::get<ATARegionState<ActionType>>(configuration);
			               RegionIndex &region_index = std::get<1>(ata_configuration);
			               // Increment if the region index is less than the max_region_index and if the
			               // region index is even or if we increment all region indexes.
			               if (region_index < max_region_index) {
				               region_index += 1;
			               }
		               }
		               return configuration;
	               });
	return res;
}

/** Get the CanonicalABWord that directly follows the given word. The next word
 * is the word Abs where the Abs_i with the maximal fractional part is
 * incremented such that it goes into the next region. This corresponds to
 * increasing the clock value with the maximal fractional part such that it
 * reaches the next region.
 * @param word The word for which to compute the time successor
 * @param K The upper bound for all constants appearing in clock constraints
 * @return A CanonicalABWord that directly follows the given word time-wise,
 * i.e., all Abs_i in the word Abs are the same except the last component,
 * which is incremented to the next region.
 */
template <typename Location, typename ActionType>
CanonicalABWord<Location, ActionType>
get_time_successor(const CanonicalABWord<Location, ActionType> &word, automata::ta::Integer K)
{
	assert(is_valid_canonical_word(word));
	if (word.empty()) {
		return {};
	}
	CanonicalABWord<Location, ActionType> res;
	const RegionIndex                     max_region_index = 2 * K + 1;
	// Find the last partition where at least one configuration has a region index smaller than the
	// max region index.
	auto last_nonmax_partition =
	  std::find_if(word.rbegin(), word.rend(), [&max_region_index](const auto &partition) {
		  return std::any_of(partition.begin(),
		                     partition.end(),
		                     [&max_region_index](const auto &configuration) {
			                     return get_region_index(configuration) != max_region_index;
		                     });
	  });
	// All region indexes already are the max index, nothing to increment.
	if (last_nonmax_partition == word.rend()) {
		return word;
	}
	// Split the last nonmax partition into a part that contains the nonmaxed elements and a part that
	// contains the maxed elements. We need to separate them as we only increase the region index of
	// the former, which changes the fractional part.
	std::set<ABRegionSymbol<Location, ActionType>> nonmaxed;
	std::set<ABRegionSymbol<Location, ActionType>> maxed;
	for (const auto &configuration : *last_nonmax_partition) {
		if (get_region_index(configuration) == max_region_index) {
			maxed.insert(configuration);
		} else {
			nonmaxed.insert(configuration);
		}
	}

	// The nonmaxed elements of the last nonmax partition now become the first partition (Abs_1)
	// because we increase its clocks to the next integer. If this partition's regions are all even,
	// increment all odd region indexes, as we increase the clocks by some epsilon such that they
	// reach the next integer.
	// TODO(morxa) In the latter case, we know that we have a singleton, as the maximal fractional
	// part is 0, which is also the minimal fractional part.
	if (!nonmaxed.empty()) {
		res.push_back(increment_region_indexes(nonmaxed, max_region_index));
	}
	// All the elements between last_nonmax_partition  and the last Abs_i are
	// copied without modification.
	// TODO(morxa) Isn't this the wrong order of the elements after and before last_nonmax?
	std::reverse_copy(std::rbegin(word), last_nonmax_partition, std::back_inserter(res));
	// Process all Abs_i before last_nonmax_partition.
	if (std::prev(std::rend(word)) != last_nonmax_partition) {
		// The first set needs to be incremented if its region indexes are even.
		if (get_region_index(*word.begin()->begin()) % 2 == 0) {
			res.push_back(increment_region_indexes(*word.begin(), max_region_index));
		} else {
			res.push_back(*word.begin());
		}
		// Copy all other abs_i which are not the first nor the last. Those never change.
		std::reverse_copy(std::next(last_nonmax_partition),
		                  std::prev(word.rend()),
		                  std::back_inserter(res));
	}
	if (!maxed.empty()) {
		res.push_back(std::move(maxed));
	}
	assert(is_valid_canonical_word(res));
	return res;
}

/** Get the canonical word H(s) for the given A/B configuration s, closely
 * following Bouyer et al., 2006. The TAStates of s are first expanded into
 * triples (location, clock, valuation) (one for each clock), and then merged
 * with the pairs from the ATAConfiguration. The resulting set is then
 * partitioned according to the fractional part of the clock valuations. Then,
 * each tuple is regionalized by replacing the clock valuation with the
 * respective region index.  The resulting word is a sequence of sets, each set
 * containing regionalized tuples that describe a TAState or ATAState. The
 * sequence is sorted by the fractional part of the original clock valuations.
 * @param ta_configuration The configuration of the timed automaton A
 * @param ata_configuration The configuration of the alternating timed automaton B
 * @param K The value of the largest constant any clock may be compared to
 * @return The canonical word representing the state s, as a sorted vector of
 * sets of tuples (triples from A and pairs from B).
 */
template <typename Location, typename ActionType>
CanonicalABWord<Location, ActionType>
get_canonical_word(const automata::ta::Configuration<Location> &ta_configuration,
                   const ATAConfiguration<ActionType> &         ata_configuration,
                   const unsigned int                           K)
{
	// TODO Also accept a TA that does not have any clocks.
	if (ta_configuration.second.empty()) {
		throw std::invalid_argument("TA without clocks are not supported");
	}
	std::set<ABSymbol<Location, ActionType>> g;
	// Insert ATA configurations into g.
	std::copy(ata_configuration.begin(), ata_configuration.end(), std::inserter(g, g.end()));
	// Insert TA configurations into g.
	for (const auto &[clock_name, clock_value] : ta_configuration.second) {
		g.insert(std::make_tuple(ta_configuration.first, clock_name, clock_value));
	}
	// Sort into partitions by the fractional parts.
	std::map<ClockValuation, std::set<ABSymbol<Location, ActionType>>> partitioned_g;
	for (const ABSymbol<Location, ActionType> &symbol : g) {
		partitioned_g[utilities::getFractionalPart<int, ClockValuation>(get_time(symbol))].insert(
		  symbol);
	}
	// Replace exact clock values by region indices.
	automata::ta::TimedAutomatonRegions   regionSet{K};
	CanonicalABWord<Location, ActionType> abs;
	for (const auto &[fractional_part, g_i] : partitioned_g) {
		std::set<ABRegionSymbol<Location, ActionType>> abs_i;
		std::transform(
		  g_i.begin(),
		  g_i.end(),
		  std::inserter(abs_i, abs_i.end()),
		  [&](const ABSymbol<Location, ActionType> &w) -> ABRegionSymbol<Location, ActionType> {
			  if (std::holds_alternative<TAState<Location>>(w)) {
				  const TAState<Location> &s = std::get<TAState<Location>>(w);
				  return TARegionState<Location>(std::get<0>(s),
				                                 std::get<1>(s),
				                                 regionSet.getRegionIndex(std::get<2>(s)));
			  } else {
				  const ATAState<ActionType> &s = std::get<ATAState<ActionType>>(w);
				  return ATARegionState<ActionType>(s.first, regionSet.getRegionIndex(s.second));
			  }
		  });
		abs.push_back(abs_i);
	}
	assert(is_valid_canonical_word(abs));
	return abs;
}

/**
 * @brief Get a concrete candidate-state for a given valid canonical word. The
 * candidate consists of a concrete TA-state and a concrete ATA-state.
 * @tparam Location The location type for timed automata locations
 * @tparam ActionType The type of actions which encode locations in the ATA
 * @param word The passed canonical word for which a candidate should be generated
 * @return std::pair<TAConfiguration<Location>, ATAConfiguration<ActionType>> A pair of TA- and
 * ATA-state which can be represented by the passed canonical word
 */
template <typename Location, typename ActionType>
std::pair<TAConfiguration<Location>, ATAConfiguration<ActionType>>
get_candidate(const CanonicalABWord<Location, ActionType> &word)
{
	assert(is_valid_canonical_word(word));
	TAConfiguration<Location>    ta_configuration{};
	ATAConfiguration<ActionType> ata_configuration{};
	const Time                   time_delta = Time(1) / Time(word.size() + 1);
	for (std::size_t i = 0; i < word.size(); i++) {
		const auto &abs_i = word[i];
		for (const ABRegionSymbol<Location, ActionType> &symbol : abs_i) {
			// TODO Refactor, fractional/integral outside of if
			if (std::holds_alternative<TARegionState<Location>>(symbol)) {
				const auto &      ta_region_state = std::get<TARegionState<Location>>(symbol);
				const RegionIndex region_index    = std::get<2>(ta_region_state);
				const Time        fractional_part = region_index % 2 == 0 ? 0 : time_delta * (i + 1);
				const Time        integral_part   = static_cast<RegionIndex>(region_index / 2);
				const auto &      clock_name      = std::get<1>(ta_region_state);
				// update ta_configuration
				ta_configuration.first              = std::get<0>(ta_region_state);
				ta_configuration.second[clock_name] = integral_part + fractional_part;
			} else { // ATARegionState<ActionType>
				const auto &      ata_region_state = std::get<ATARegionState<ActionType>>(symbol);
				const RegionIndex region_index     = std::get<1>(ata_region_state);
				const Time        fractional_part  = region_index % 2 == 0 ? 0 : time_delta * (i + 1);
				const Time        integral_part    = static_cast<RegionIndex>(region_index / 2);
				// update configuration
				// TODO check: the formula (aka ActionType) encodes the location, the clock valuation is
				// separate and a configuration is a set of such pairs. Is this already sufficient?
				ata_configuration.insert(
				  std::make_pair(std::get<0>(ata_region_state), fractional_part + integral_part));
			}
		}
	}
	return std::make_pair(ta_configuration, ata_configuration);
}

/**
 * @brief Get the next canonical words from the passed word.
 * @details A successor of a regionalized configuration in the regionalized synchronous product is
 * built from a time t >= 0 and a letter a for which there exists both a successor in A and a
 * successor in B. To compute possible successors, we need to individually compute
 * region-successors for A and B for all letters of the alphabet and for all possible time
 * durations/delays. For a single letter a, we need to find a common time interval T for which
 * both in A and B, after letting time t in T pass, a transition labeled with a is enabled. The
 * regionalized product successor is then built from the resulting regions after letting time t
 * pass and taking the transition labeled with a in both automata.
 * @tparam Location
 * @tparam ActionType
 * @param ta
 * @param ata
 * @param canonical_word
 * @param K
 * @return CanonicalABWord
 */
template <typename Location, typename ActionType>
std::vector<std::tuple<RegionIndex, ActionType, CanonicalABWord<Location, ActionType>>>
get_next_canonical_words(
  const automata::ta::TimedAutomaton<Location, ActionType> &                            ta,
  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ActionType>,
                                                 logic::AtomicProposition<ActionType>> &ata,
  CanonicalABWord<Location, ActionType> canonical_word,
  RegionIndex                           K)
{
	std::vector<std::tuple<RegionIndex, ActionType, CanonicalABWord<Location, ActionType>>> res;

	// Compute all time successors
	// TODO Refactor into a separate function
	SPDLOG_TRACE("Computing time successors of {} with K={}", canonical_word, K);
	auto                                               cur = get_time_successor(canonical_word, K);
	std::vector<CanonicalABWord<Location, ActionType>> time_successors;
	time_successors.push_back(canonical_word);
	auto &prev = canonical_word;
	// TODO merge this loop and the following loops.
	while (cur != prev) {
		time_successors.emplace_back(cur);
		prev = time_successors.back();
		cur  = get_time_successor(prev, K);
	}

	SPDLOG_TRACE("Time successors: {}", time_successors);

	// Intermediate step: Create concrete candidate for each abstract time successor (represented by
	// the respective canonical word).
	std::vector<std::pair<TAConfiguration<Location>, ATAConfiguration<ActionType>>>
	  concrete_candidates;
	concrete_candidates.reserve(time_successors.size());
	std::transform( // std::execution::seq, // Make sure we keep the order of elements.
	  time_successors.begin(),
	  time_successors.end(),
	  std::back_inserter(concrete_candidates),
	  [](const auto &word) { return get_candidate(word); });
	assert(time_successors.size() == concrete_candidates.size());

	// Compute the regionalized successors of the concrete candidats.
	for (std::size_t i = 0; i < concrete_candidates.size(); ++i) {
		const auto ab_configuration = concrete_candidates[i];
		// Try to make a symbol step for each symbol in the alphabet.
		for (const auto symbol : ta.get_alphabet()) {
			const std::set<TAConfiguration<Location>> ta_successors =
			  ta.make_symbol_step(ab_configuration.first, symbol);
			const std::set<ATAConfiguration<ActionType>> ata_successors =
			  ata.make_symbol_step(ab_configuration.second, symbol);
			// For all successors, compute the canonical word and add it to the result.
			for (const auto &ta_successor : ta_successors) {
				for (const auto &ata_successor : ata_successors) {
					// The index is is also the number of time steps we do, i.e., the region increment.
					res.push_back(
					  std::make_tuple(i, symbol, get_canonical_word(ta_successor, ata_successor, K)));
					SPDLOG_TRACE("Getting {} from ({}, {}) with symbol {}",
					             res.back(),
					             ab_configuration.first,
					             ab_configuration.second,
					             symbol);
				}
			}
		}
	}

	return res;
}

} // namespace synchronous_product
