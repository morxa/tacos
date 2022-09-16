/***************************************************************************
 *  synchronous_product.h - The synchronous product of a TA and an ATA
 *
 *  Created:   Mon 21 Dec 16:13:49 CET 2020
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include "automata/ata.h"
#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_regions.h"
#include "canonical_word.h"
#include "mtl/MTLFormula.h"
#include "utilities/numbers.h"
#include "utilities/types.h"

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace tacos::search {

/// Increment the region indexes in the configurations of the given ABRegionSymbol.
/** This is a helper function to increase the region index so we reach the next region set.
 * @param configurations The set of configurations to increment the region indexes in
 * @param max_region_index The maximal region index that may never be passed
 * @return A copy of the given configurations with incremented region indexes
 */
template <typename Location, typename ConstraintSymbolType>
std::set<ABRegionSymbol<Location, ConstraintSymbolType>>
increment_region_indexes(
  const std::set<ABRegionSymbol<Location, ConstraintSymbolType>> &configurations,
  RegionIndex                                                     max_region_index)
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
	std::set<ABRegionSymbol<Location, ConstraintSymbolType>> res;
	std::transform(configurations.begin(),
	               configurations.end(),
	               std::inserter(res, res.end()),
	               [max_region_index](auto configuration) {
		               if (std::holds_alternative<PlantRegionState<Location>>(configuration)) {
			               auto &ta_configuration = std::get<PlantRegionState<Location>>(configuration);
			               RegionIndex &region_index = ta_configuration.region_index;
			               // Increment if the region index is less than the max_region_index and if the
			               // region index is even or if we increment all region indexes.
			               if (region_index < max_region_index) {
				               region_index += 1;
			               }
		               } else {
			               auto &ata_configuration =
			                 std::get<ATARegionState<ConstraintSymbolType>>(configuration);
			               RegionIndex &region_index = ata_configuration.region_index;
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
template <typename Location, typename ConstraintSymbolType>
CanonicalABWord<Location, ConstraintSymbolType>
get_time_successor(const CanonicalABWord<Location, ConstraintSymbolType> &word, RegionIndex K)
{
	if (word.empty()) {
		return {};
	}
	CanonicalABWord<Location, ConstraintSymbolType> res;
	const RegionIndex                               max_region_index = 2 * K + 1;
	assert(is_valid_canonical_word(word, max_region_index));
	// Find the partition that contains all maxed partitions. If it does not exist, create an empty
	// one.
	std::set<ABRegionSymbol<Location, ConstraintSymbolType>> new_maxed_partition;
	auto last_nonmaxed_partition = std::next(word.rbegin());
	// Check if maxed partition is actually maxed
	if (std::all_of(word.rbegin()->begin(),
	                word.rbegin()->end(),
	                [&max_region_index](const auto &configuration) {
		                return get_region_index(configuration) == max_region_index;
	                })) {
		new_maxed_partition = *word.rbegin();
	} else {
		// There is no maxed partition, so the last partition is already nonmaxed.
		last_nonmaxed_partition = word.rbegin();
	}
	if (last_nonmaxed_partition == word.rend()) {
		// All partitions are maxed, nothing to increment.
		return word;
	}
	{
		// Increment the last nonmaxed partition. If we have a new maxed configuration, put it into the
		// maxed partition. Otherwise, keep it in place.
		auto incremented = increment_region_indexes(*last_nonmaxed_partition, max_region_index);
		std::set<ABRegionSymbol<Location, ConstraintSymbolType>> incremented_nonmaxed;
		for (auto &configuration : incremented) {
			if (get_region_index(configuration) == max_region_index) {
				new_maxed_partition.insert(configuration);
			} else {
				incremented_nonmaxed.insert(configuration);
			}
		}
		if (!incremented_nonmaxed.empty()) {
			res.push_back(std::move(incremented_nonmaxed));
		}
	}

	// Process all partitions before the last nonmaxed partition.
	if (std::prev(std::rend(word)) != last_nonmaxed_partition) {
		// The first set needs to be incremented if its region indexes are even.
		if (get_region_index(*word.begin()->begin()) % 2 == 0) {
			auto incremented = increment_region_indexes(*word.begin(), max_region_index);
			std::set<ABRegionSymbol<Location, ConstraintSymbolType>> incremented_nonmaxed;
			for (auto &configuration : incremented) {
				if (get_region_index(configuration) == max_region_index) {
					new_maxed_partition.insert(configuration);
				} else {
					incremented_nonmaxed.insert(configuration);
				}
			}
			if (!incremented_nonmaxed.empty()) {
				res.push_back(std::move(incremented_nonmaxed));
			}
		} else {
			res.push_back(*word.begin());
		}
		// Copy all other abs_i which are not the first nor the last. Those never change.
		std::reverse_copy(std::next(last_nonmaxed_partition),
		                  std::prev(word.rend()),
		                  std::back_inserter(res));
	}
	// If the maxed partition is non-empty, add it to the resulting word.
	if (!new_maxed_partition.empty()) {
		res.push_back(std::move(new_maxed_partition));
	}
	assert(is_valid_canonical_word(res, max_region_index));
	return res;
}

/**
 * @brief Get a concrete candidate-state for a given valid canonical word. The
 * candidate consists of a concrete TA-state and a concrete ATA-state.
 * @tparam Location The location type for timed automata locations
 * @tparam ConstraintSymbolType The type of actions which encode locations in the ATA
 * @param word The passed canonical word for which a candidate should be generated
 * @return std::pair<TAConfiguration<Location>, ATAConfiguration<ConstraintSymbolType>> A pair of
 * TA- and ATA-state which can be represented by the passed canonical word
 */
template <typename Location /* TA::Location */, typename ConstraintSymbolType>
std::pair<PlantConfiguration<Location>, ATAConfiguration<ConstraintSymbolType>>
get_candidate(const CanonicalABWord<Location, ConstraintSymbolType> &word)
{
	assert(is_valid_canonical_word(word));
	PlantConfiguration<Location>           plant_configuration{};
	ATAConfiguration<ConstraintSymbolType> ata_configuration{};
	const Time                             time_delta = Time(1) / Time(word.size() + 1);
	for (std::size_t i = 0; i < word.size(); i++) {
		const auto &abs_i = word[i];
		for (const ABRegionSymbol<Location, ConstraintSymbolType> &symbol : abs_i) {
			// TODO Refactor, fractional/integral outside of if
			if (std::holds_alternative<PlantRegionState<Location>>(symbol)) {
				const auto       &ta_region_state = std::get<PlantRegionState<Location>>(symbol);
				const RegionIndex region_index    = ta_region_state.region_index;
				const Time        fractional_part =
          region_index % 2 == 0 ? 0 : time_delta * static_cast<Time>((i + 1));
				const Time  integral_part = static_cast<RegionIndex>(region_index / 2);
				const auto &clock_name    = ta_region_state.clock;
				// update ta_configuration
				plant_configuration.location                     = ta_region_state.location;
				plant_configuration.clock_valuations[clock_name] = integral_part + fractional_part;
			} else { // ATARegionState<ConstraintSymbolType>
				const auto       &ata_region_state = std::get<ATARegionState<ConstraintSymbolType>>(symbol);
				const RegionIndex region_index     = ata_region_state.region_index;
				const Time        fractional_part =
          region_index % 2 == 0 ? 0 : time_delta * static_cast<Time>((i + 1));
				const Time integral_part = static_cast<RegionIndex>(region_index / 2);
				// update configuration
				// TODO check: the formula (aka ConstraintSymbolType) encodes the location, the clock
				// valuation is separate and a configuration is a set of such pairs. Is this already
				// sufficient?
				ata_configuration.insert(ATAState<ConstraintSymbolType>{ata_region_state.formula,
				                                                        fractional_part + integral_part});
			}
		}
	}
	return std::make_pair(plant_configuration, ata_configuration);
}

/** Get the nth time successor. */
template <typename Location, typename ConstraintSymbolType>
CanonicalABWord<Location, ConstraintSymbolType>
get_nth_time_successor(const CanonicalABWord<Location, ConstraintSymbolType> &word,
                       RegionIndex                                            n,
                       RegionIndex                                            K)
{
	auto res = word;
	for (RegionIndex i = 0; i < n; i++) {
		res = get_time_successor(res, K);
	}
	return res;
}

/** Compute all time successors of a canonical word.
 * @param canonical_word The canonical word to compute the time successors of
 * @param K The maximal constant
 * @return All time successors of the word along with the region increment to reach the successor
 */
template <typename Location, typename ConstraintSymbolType>
std::vector<std::pair<RegionIndex, CanonicalABWord<Location, ConstraintSymbolType>>>
get_time_successors(const CanonicalABWord<Location, ConstraintSymbolType> &canonical_word,
                    RegionIndex                                            K)
{
	SPDLOG_TRACE("Computing time successors of {} with K={}", canonical_word, K);
	auto        cur = get_time_successor(canonical_word, K);
	RegionIndex cur_index{0};
	std::vector<std::pair<RegionIndex, CanonicalABWord<Location, ConstraintSymbolType>>>
	  time_successors;
	time_successors.push_back(std::make_pair(cur_index++, canonical_word));
	for (; cur != time_successors.back().second; cur_index++) {
		time_successors.emplace_back(cur_index, cur);
		cur = get_time_successor(time_successors.back().second, K);
	}
	return time_successors;
}

/** Compute all time successors of a set of canonical words (i.e., of a node in the search tree).
 * @param canonical_words A set of canonical words to compute the time successors of
 * @param K The maximal constant
 * @return A map of time successors of each word along with the region increment to reach the
 * successor
 */
template <typename Location, typename ConstraintSymbolType>
std::map<CanonicalABWord<Location, ConstraintSymbolType>,
         std::vector<std::pair<RegionIndex, CanonicalABWord<Location, ConstraintSymbolType>>>>
get_time_successors(
  const std::set<CanonicalABWord<Location, ConstraintSymbolType>> &canonical_words,
  RegionIndex                                                      K)
{
	std::map<CanonicalABWord<Location, ConstraintSymbolType>,
	         std::vector<std::pair<RegionIndex, CanonicalABWord<Location, ConstraintSymbolType>>>>
	            res;
	std::size_t num_successors = 0;
	for (const auto &word : canonical_words) {
		const auto successors = get_time_successors(word, K);
		num_successors        = std::max(num_successors, successors.size());
		res[word]             = successors;
	}
	for (auto &&[_, successors] : res) {
		for (std::size_t i = successors.size(); i < num_successors; ++i) {
			successors.push_back(std::make_pair(i, successors.back().second));
		}
	}
	return res;
}
} // namespace tacos::search
