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
#include "mtl/MTLFormula.h"
#include "utilities/numbers.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <variant>

/** Get the regionalized synchronous product of a TA and an ATA. */
namespace synchronous_product {

using automata::ClockValuation;
using automata::Time;
using automata::ta::RegionIndex;
template <typename Location>
/** Short-hand type alias for a configuration of a TA */
using TAConfiguration = automata::ta::Configuration<Location>;
/** Always use ATA configurations over MTLFormulas */
template <typename ActionType>
using ATAConfiguration = automata::ata::Configuration<logic::MTLFormula<ActionType>>;
/** An expanded state (location, clock_name, clock_valuation) of a TimedAutomaton */
template <typename Location>
using TAState = std::tuple<Location, std::string, ClockValuation>;
/** Always use ATA states over MTLFormulas */
template <typename ActionType>
using ATAState = automata::ata::State<logic::MTLFormula<ActionType>>;
/** An ABSymbol is either a TAState or an ATAState */
template <typename Location, typename ActionType>
using ABSymbol = std::variant<TAState<Location>, ATAState<ActionType>>;
/** A TARegionState is a tuple (location, clock_name, clock_region) */
template <typename Location>
using TARegionState = std::tuple<Location, std::string, RegionIndex>;
/** An ATARegionState is a pair (formula, clock_region) */
template <typename ActionType>
using ATARegionState = std::pair<logic::MTLFormula<ActionType>, RegionIndex>;
/** An ABRegionSymbol is either a TARegionState or an ATARegionState */
template <typename Location, typename ActionType>
using ABRegionSymbol = std::variant<TARegionState<Location>, ATARegionState<ActionType>>;

/** A canonical word H(s) for a regionalized A/B configuration */
template <typename Location, typename ActionType>
using CanonicalABWord = std::vector<std::set<ABRegionSymbol<Location, ActionType>>>;

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

	/** Construct the exception from a parameter pack
	 * @param args The error message as a parameter pack, all parameters must be streamable to an
	 * ostream
	 */
	template <typename... Args>
	explicit InvalidCanonicalWordException(Args &&...args) : std::domain_error("")
	{
		std::stringstream error;
		(error << ... << args);
		message_ = error.str();
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
	if (word.empty()) {
		throw InvalidCanonicalWordException("Word ", word, " is empty");
	}
	// TODO all ta_symbols should agree on the same location
	// No configuration should be empty
	if (std::any_of(word.begin(), word.end(), [](const auto &configurations) {
		    return configurations.empty();
	    })) {
		throw InvalidCanonicalWordException("Word ", word, " contains no configuration");
	}
	// Each partition either contains only even or only odd region indexes. This is because the word
	// is partitioned by the fractional part and the region index can only be even if the fractional
	// part is 0. If that is the case, there cannot be any configuration with an odd region index in
	// the same partition, as that configuration's fractional part would be > 0.
	std::for_each(word.begin(), word.end(), [](const auto &configurations) {
		if (std::any_of(configurations.begin(),
		                configurations.end(),
		                [](const auto &w) { return get_region_index(w) % 2 == 0; })
		    && std::any_of(configurations.begin(), configurations.end(), [](const auto &w) {
			       return get_region_index(w) % 2 == 1;
		       })) {
			throw InvalidCanonicalWordException("Inconsistent regions in ",
			                                    configurations,
			                                    ": both odd and even region indexes");
		}
	});
	// There must be at most one partition with fractional part 0.
	// The only partition that is allowed to have fracitonal part 0 is the 0th
	// partition.
	std::for_each(std::next(word.begin()), word.end(), [](const auto &configurations) {
		std::for_each(configurations.begin(), configurations.end(), [&configurations](const auto &w) {
			if (get_region_index(w) % 2 == 0) {
				throw InvalidCanonicalWordException("Fractional part 0 in wrong element ",
				                                    w,
				                                    " of partition ",
				                                    configurations);
			}
		});
	});
	return true;
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
 * @brief Get the next canonical words based on the passed regionalized configuration
 * @details Computes the next canonical words in the regionalized synchronous product quotient based
 * on the current configuration. To do this, we compute the canonical word associated and then a
 * possible successor from this is returned.
 * @tparam Location
 * @tparam ActionType
 * @param ta_configuration
 * @param ata_configuration
 * @param K
 * @return std::vector<std::set<ABRegionSymbol<Location, ActionType>>>
 */
template <typename Location, typename ActionType>
std::vector<std::vector<std::set<ABRegionSymbol<Location, ActionType>>>>
get_next_canonical_words(const automata::ta::Configuration<Location> &ta_configuration,
                         const ATAConfiguration<ActionType> &         ata_configuration,
                         const unsigned int                           K)
{
	return get_next_canonical_words(get_canonical_word(ta_configuration, ata_configuration, K));
}

/**
 * @brief Get the next canonical words from the passed word.
 * @details A successor of a regionalized configuration in the regionalized synchronous product is
 * built from a time t >= 0 and a letter a for which there exists both a successor in A and a
 * successor in B. To compute possible successors, we need to individually compute region-successors
 * for A and B for all letters of the alphabet and for all possible time durations/delays. For a
 * single letter a, we need to find a common time interval T for which both in A and B, after
 * letting time t in T pass, a transition labeled with a is enabled. The regionalized product
 * successor is then built from the resulting regions after letting time t pass and taking the
 * transition labeled with a in both automata.
 * @tparam Location
 * @tparam ActionType
 * @param canonical_word
 * @return std::vector<std::set<ABRegionSymbol<Location, ActionType>>>
 */
template <typename Location, typename ActionType>
std::vector<std::vector<std::set<ABRegionSymbol<Location, ActionType>>>>
get_next_canonical_words(
  const std::vector<std::set<ABRegionSymbol<Location, ActionType>>> &canonical_word)
{
	std::vector<std::vector<std::set<ABRegionSymbol<Location, ActionType>>>> res;
	// TODO Compute all time successors
	// TODO For each time successor, compute TA successors
	// TODO Filter with ATA successors
	// TODO Transform back into canonical word

	return res;
}

/// Increment the region indexes in the configurations of the given ABRegionSymbol.
/** By default, increment the region index of each configuration if the region index is even.
 * Optionally, also increase odd region indexes, but only if there is no configuration with an even
 * region index. This is a helper function to increase the region index so we reach the next region
 * set. For an even region index, we need to increment the region index, because if we increase the
 * clocks by some epsilon, we already reach the next region. For an odd region index, we only need
 * to increase the index if we increment the clocks such that we reach the next region. This is only
 * the case if the fractional part of the clock valuations is maximal compared to the other
 * ABRegionSymbols, i.e., if the given region symbol is the last in the CanonicalABWord.
 * @param configurations The set of configurations to increment the region indexes in
 * @param max_region_index The maximal region index that may never be passed
 * @param increment_odd_indexes If true, also increment odd region indexes, but only if there is no
 * even region index.
 * @return A copy of the given configurations with incremented region indexes
 */
template <typename Location, typename ActionType>
std::set<ABRegionSymbol<Location, ActionType>>
increment_region_indexes(const std::set<ABRegionSymbol<Location, ActionType>> &configurations,
                         RegionIndex                                           max_region_index,
                         bool increment_odd_indexes = false)
{
	bool always_increment =
	  increment_odd_indexes
	  && !std::any_of(configurations.begin(), configurations.end(), [](const auto &configuration) {
		     return get_region_index(configuration) % 2 == 0;
	     });
	std::set<ABRegionSymbol<Location, ActionType>> res;
	std::transform(configurations.begin(),
	               configurations.end(),
	               std::inserter(res, res.end()),
	               [max_region_index, always_increment](auto configuration) {
		               if (std::holds_alternative<TARegionState<Location>>(configuration)) {
			               auto &ta_configuration    = std::get<TARegionState<Location>>(configuration);
			               RegionIndex &region_index = std::get<2>(ta_configuration);
			               // Increment if the region index is less than the max_region_index and if the
			               // region index is even or if we increment all region indexes.
			               if (region_index < max_region_index
			                   && (always_increment || region_index % 2 == 0)) {
				               region_index += 1;
			               }
		               } else {
			               auto &ata_configuration = std::get<ATARegionState<ActionType>>(configuration);
			               RegionIndex &region_index = std::get<1>(ata_configuration);
			               // Increment if the region index is less than the max_region_index and if the
			               // region index is even or if we increment all region indexes.
			               if (region_index < max_region_index
			                   && (always_increment || region_index % 2 == 0)) {
				               region_index += 1;
			               }
		               }
		               return configuration;
	               });
	return res;
};

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
	// The last nonmax partition now becomes the first partition (Abs_1) because we increase its
	// clocks to the next integer. If this partition's regions are all even, increment all odd
	// region indexes, as we increase the clocks by some epsilon such that they reach the next
	// integer.
	// TODO In the latter case, we know that we have a singleton, as the maximal fractional part is
	// 0, which is also the minimal fractional part.
	res.push_back(increment_region_indexes(*last_nonmax_partition, max_region_index, true));
	// All the elements between last_nonmax_partition  and the last Abs_i are
	// copied without modification.
	std::reverse_copy(std::rbegin(word), last_nonmax_partition, std::back_inserter(res));
	// Process all Abs_i before last_nonmax_partition.
	if (std::prev(std::rend(word)) != last_nonmax_partition) {
		// The first set needs to be incremented if its region indexes are even.
		res.push_back(increment_region_indexes(*word.begin(), max_region_index));
		// Copy all other abs_i which are not the first nor the last. Those never change.
		std::reverse_copy(std::next(last_nonmax_partition),
		                  std::prev(word.rend()),
		                  std::back_inserter(res));
	}
	assert(is_valid_canonical_word(res));
	return res;
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
				const auto &      ata_region_state = std::get<ATARegionState<Location>>(symbol);
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

} // namespace synchronous_product

template <typename Location>
std::ostream &
operator<<(std::ostream &os, const synchronous_product::TARegionState<Location> &state)
{
	os << "(" << std::get<0>(state) << ", " << std::get<1>(state) << ", " << std::get<2>(state)
	   << ")";
	return os;
}

template <typename ActionType>
std::ostream &
operator<<(std::ostream &os, const synchronous_product::ATARegionState<ActionType> &state)
{
	os << "(" << state.first << ", " << state.second << ")";
	return os;
}

template <typename Location, typename ActionType>
std::ostream &
operator<<(std::ostream &                                                   os,
           const synchronous_product::ABRegionSymbol<Location, ActionType> &symbol)
{
	std::visit([&os](const auto &v) { os << v; }, symbol);
	return os;
}

template <typename Location, typename ActionType>
std::ostream &
operator<<(std::ostream &                                                             os,
           const std::set<synchronous_product::ABRegionSymbol<Location, ActionType>> &word)
{
	if (word.empty()) {
		os << "{}";
		return os;
	}
	os << "{ ";
	bool first = true;
	for (const auto &symbol : word) {
		if (!first) {
			os << ", ";
		} else {
			first = false;
		}
		os << symbol;
	}
	os << " }";
	return os;
}

template <typename Location, typename ActionType>
std::ostream &
operator<<(
  std::ostream &                                                                          os,
  const std::vector<std::set<synchronous_product::ABRegionSymbol<Location, ActionType>>> &word)
{
	if (word.empty()) {
		os << "[]";
		return os;
	}
	os << "[ ";
	bool first = true;
	for (const auto &symbol : word) {
		if (!first) {
			os << ", ";
		} else {
			first = false;
		}
		os << symbol;
	}
	os << " ]";
	return os;
}
