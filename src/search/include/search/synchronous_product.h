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

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace tacos::search {

/** Get the clock valuation for an ABSymbol, which is either a TA state or an ATA state.
 * @param w The symbol to read the time from
 * @return The clock valuation in the given state
 */
template <typename Location, typename ConstraintSymbolType>
ClockValuation
get_time(const ABSymbol<Location, ConstraintSymbolType> &w)
{
	if (std::holds_alternative<TAState<Location>>(w)) {
		return std::get<TAState<Location>>(w).clock_valuation;
	} else {
		return std::get<ATAState<ConstraintSymbolType>>(w).clock_valuation;
	}
}

/** Get the clock valuation for an ABRegionSymbol, which is either a
 * TARegionState or an ATARegionState.
 * @param w The symbol to read the time from
 * @return The region index in the given state
 */
template <typename Location, typename ConstraintSymbolType>
RegionIndex
get_region_index(const ABRegionSymbol<Location, ConstraintSymbolType> &w)
{
	if (std::holds_alternative<TARegionState<Location>>(w)) {
		return std::get<TARegionState<Location>>(w).region_index;
	} else {
		return std::get<ATARegionState<ConstraintSymbolType>>(w).region_index;
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
	template <typename Location, typename ConstraintSymbolType>
	explicit InvalidCanonicalWordException(
	  const CanonicalABWord<Location, ConstraintSymbolType> &word,
	  const std::string &                                    error)
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
template <typename Location, typename ConstraintSymbolType>
bool
is_valid_canonical_word(const CanonicalABWord<Location, ConstraintSymbolType> &word)
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
		               if (std::holds_alternative<TARegionState<Location>>(configuration)) {
			               auto &ta_configuration    = std::get<TARegionState<Location>>(configuration);
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
get_time_successor(const CanonicalABWord<Location, ConstraintSymbolType> &word,
                   automata::ta::Integer                                  K)
{
	assert(is_valid_canonical_word(word));
	if (word.empty()) {
		return {};
	}
	CanonicalABWord<Location, ConstraintSymbolType> res;
	const RegionIndex                               max_region_index = 2 * K + 1;
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
template <typename Location, typename ConstraintSymbolType>
CanonicalABWord<Location, ConstraintSymbolType>
get_canonical_word(const automata::ta::Configuration<Location> & ta_configuration,
                   const ATAConfiguration<ConstraintSymbolType> &ata_configuration,
                   const unsigned int                            K)
{
	// TODO Also accept a TA that does not have any clocks.
	if (ta_configuration.clock_valuations.empty()) {
		throw std::invalid_argument("TA without clocks are not supported");
	}
	std::set<ABSymbol<Location, ConstraintSymbolType>> g;
	// Insert ATA configurations into g.
	std::copy(ata_configuration.begin(), ata_configuration.end(), std::inserter(g, g.end()));
	// Insert TA configurations into g.
	for (const auto &[clock_name, clock_value] : ta_configuration.clock_valuations) {
		g.insert(TAState<Location>{ta_configuration.location, clock_name, clock_value});
	}
	// Sort into partitions by the fractional parts.
	std::map<ClockValuation,
	         std::set<ABSymbol<Location, ConstraintSymbolType>>,
	         utilities::ApproxFloatComparator<Time>>
	  partitioned_g(utilities::ApproxFloatComparator<Time>{});
	for (const ABSymbol<Location, ConstraintSymbolType> &symbol : g) {
		partitioned_g[utilities::getFractionalPart<int, ClockValuation>(get_time(symbol))].insert(
		  symbol);
	}
	// Replace exact clock values by region indices.
	automata::ta::TimedAutomatonRegions             regionSet{K};
	CanonicalABWord<Location, ConstraintSymbolType> abs;
	for (const auto &[fractional_part, g_i] : partitioned_g) {
		std::set<ABRegionSymbol<Location, ConstraintSymbolType>> abs_i;
		std::transform(g_i.begin(),
		               g_i.end(),
		               std::inserter(abs_i, abs_i.end()),
		               [&](const ABSymbol<Location, ConstraintSymbolType> &w)
		                 -> ABRegionSymbol<Location, ConstraintSymbolType> {
			               if (std::holds_alternative<TAState<Location>>(w)) {
				               const TAState<Location> &s = std::get<TAState<Location>>(w);
				               return TARegionState<Location>{s.location,
				                                              s.clock,
				                                              regionSet.getRegionIndex(s.clock_valuation)};
			               } else {
				               const ATAState<ConstraintSymbolType> &s =
				                 std::get<ATAState<ConstraintSymbolType>>(w);
				               return ATARegionState<ConstraintSymbolType>{s.location,
				                                                           regionSet.getRegionIndex(
				                                                             s.clock_valuation)};
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
 * @tparam ConstraintSymbolType The type of actions which encode locations in the ATA
 * @param word The passed canonical word for which a candidate should be generated
 * @return std::pair<TAConfiguration<Location>, ATAConfiguration<ConstraintSymbolType>> A pair of
 * TA- and ATA-state which can be represented by the passed canonical word
 */
template <typename Location, typename ConstraintSymbolType>
std::pair<TAConfiguration<Location>, ATAConfiguration<ConstraintSymbolType>>
get_candidate(const CanonicalABWord<Location, ConstraintSymbolType> &word)
{
	assert(is_valid_canonical_word(word));
	TAConfiguration<Location>              ta_configuration{};
	ATAConfiguration<ConstraintSymbolType> ata_configuration{};
	const Time                             time_delta = Time(1) / Time(word.size() + 1);
	for (std::size_t i = 0; i < word.size(); i++) {
		const auto &abs_i = word[i];
		for (const ABRegionSymbol<Location, ConstraintSymbolType> &symbol : abs_i) {
			// TODO Refactor, fractional/integral outside of if
			if (std::holds_alternative<TARegionState<Location>>(symbol)) {
				const auto &      ta_region_state = std::get<TARegionState<Location>>(symbol);
				const RegionIndex region_index    = ta_region_state.region_index;
				const Time        fractional_part = region_index % 2 == 0 ? 0 : time_delta * (i + 1);
				const Time        integral_part   = static_cast<RegionIndex>(region_index / 2);
				const auto &      clock_name      = ta_region_state.clock;
				// update ta_configuration
				ta_configuration.location                     = ta_region_state.location;
				ta_configuration.clock_valuations[clock_name] = integral_part + fractional_part;
			} else { // ATARegionState<ConstraintSymbolType>
				const auto &      ata_region_state = std::get<ATARegionState<ConstraintSymbolType>>(symbol);
				const RegionIndex region_index     = ata_region_state.region_index;
				const Time        fractional_part  = region_index % 2 == 0 ? 0 : time_delta * (i + 1);
				const Time        integral_part    = static_cast<RegionIndex>(region_index / 2);
				// update configuration
				// TODO check: the formula (aka ConstraintSymbolType) encodes the location, the clock
				// valuation is separate and a configuration is a set of such pairs. Is this already
				// sufficient?
				ata_configuration.insert(ATAState<ConstraintSymbolType>{ata_region_state.formula,
				                                                        fractional_part + integral_part});
			}
		}
	}
	return std::make_pair(ta_configuration, ata_configuration);
}

/** Get the nth time successor. */
template <typename Location, typename ConstraintSymbolType>
CanonicalABWord<Location, ConstraintSymbolType>
get_nth_time_successor(const CanonicalABWord<Location, ConstraintSymbolType> &word,
                       RegionIndex                                            n,
                       automata::ta::Integer                                  K)
{
	auto res = word;
	for (RegionIndex i = 0; i < n; i++) {
		res = get_time_successor(res, K);
	}
	return res;
}

/** Compute all time successors of a canonical word.
 * @param canonical_word The canonical to compute the time successors of
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

/** @brief Compute all successors for one particular time successor and one particular symbol.
 * Compute the successors by following all transitions in the TA and ATA for one time successor
 * and one symbol.
 * */
template <typename Location,
          typename ActionType,
          typename ConstraintSymbolType,
          bool use_location_constraints = false>
std::vector<CanonicalABWord<Location, ConstraintSymbolType>>
get_next_canonical_words(
  const automata::ta::TimedAutomaton<Location, ActionType> &ta,
  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ConstraintSymbolType>,
                                                 logic::AtomicProposition<ConstraintSymbolType>>
    &ata,
  const std::pair<TAConfiguration<Location>, ATAConfiguration<ConstraintSymbolType>>
    &               ab_configuration,
  const ActionType &symbol,
  RegionIndex       K)
{
	static_assert(use_location_constraints || std::is_same_v<ActionType, ConstraintSymbolType>);
	static_assert(!use_location_constraints || std::is_same_v<Location, ConstraintSymbolType>);
	std::vector<CanonicalABWord<Location, ConstraintSymbolType>> res;
	SPDLOG_TRACE("({}, {}): Symbol {}", ab_configuration.first, ab_configuration.second, symbol);
	const std::set<TAConfiguration<Location>> ta_successors =
	  ta.make_symbol_step(ab_configuration.first, symbol);
	std::set<ATAConfiguration<ConstraintSymbolType>> ata_successors;
	if constexpr (!use_location_constraints) {
		ata_successors = ata.make_symbol_step(ab_configuration.second, symbol);
	}
	SPDLOG_TRACE("({}, {}): TA successors: {} ATA successors: {}",
	             ab_configuration.first,
	             ab_configuration.second,
	             ta_successors.size(),
	             ata_successors.size());
	for (const auto &ta_successor : ta_successors) {
		SPDLOG_TRACE("({}, {}): TA successor {}",
		             ab_configuration.first,
		             ab_configuration.second,
		             ta_successor);
		if constexpr (use_location_constraints) {
			ata_successors = ata.make_symbol_step(ab_configuration.second,
			                                      logic::AtomicProposition{ta_successor.location.get()});
		}
		for (const auto &ata_successor : ata_successors) {
			SPDLOG_TRACE("({}, {}): ATA successor {}",
			             ab_configuration.first,
			             ab_configuration.second,
			             ata_successor);
			res.push_back(get_canonical_word(ta_successor, ata_successor, K));
			SPDLOG_TRACE("({}, {}): Getting {} with symbol {}",
			             ab_configuration.first,
			             ab_configuration.second,
			             res.back(),
			             symbol);
		}
	}
	return res;
}

} // namespace tacos::search
