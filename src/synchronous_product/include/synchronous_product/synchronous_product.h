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

	return res;
}

/** Get the CanonicalABWord that directly follows the given word. The next word
 * is the word Abs where the Abs_i with the maximal fractional part is
 * incremented such that it goes into the next region. This corresponds to
 * increasing the clock value with the maximal fractional part such that it
 * reaches the next region.
 * @param word The word for which to compute the time successor
 * @return A CanonicalABWord that directly follows the given word time-wise,
 * i.e., all Abs_i in the word Abs are the same except the last component,
 * which is incremented to the next region.
 */
template <typename Location, typename ActionType>
CanonicalABWord<Location, ActionType>
get_time_successor(const CanonicalABWord<Location, ActionType> &word, automata::ta::Integer K)
{
	if (word.empty()) {
		return {};
	}
	CanonicalABWord<Location, ActionType> res;
	const RegionIndex                     max_region_index = 2 * K + 1;
	// Increment the region index in the given configuration by 1.
	auto increment_region_index = [max_region_index](
	                                ABRegionSymbol<Location, ActionType> configuration) {
		if (std::holds_alternative<TARegionState<Location>>(configuration)) {
			auto &ta_configuration = std::get<TARegionState<Location>>(configuration);
			// Increment if less than the max_region_index.
			std::get<2>(ta_configuration) = std::min(max_region_index, std::get<2>(ta_configuration) + 1);
		} else {
			auto &ata_configuration = std::get<ATARegionState<ActionType>>(configuration);
			// Increment if less than the max_region_index.
			ata_configuration.second = std::min(max_region_index, ata_configuration.second + 1);
		}
		return configuration;
	};
	// Find the first partition where at least one configuration has a region index != max region
	// index.
	auto first_nonmax_partition =
	  std::find_if(word.rbegin(), word.rend(), [&max_region_index](const auto &partition) {
		  return std::any_of(partition.begin(),
		                     partition.end(),
		                     [&max_region_index](const auto &configuration) {
			                     return get_region_index(configuration) != max_region_index;
		                     });
	  });
	// All region indexes already are the max index, nothing to increment.
	if (first_nonmax_partition == word.rend()) {
		return word;
	}
	// The last Abs_i now becomes the first abs_i because we increase the clocks
	// to the next integer.
	res.push_back(std::set<ABRegionSymbol<Location, ActionType>>{});
	// The last Abs_i's region is always incremented.
	std::transform(first_nonmax_partition->begin(),
	               first_nonmax_partition->end(),
	               std::inserter(res.back(), res.back().end()),
	               increment_region_index);
	std::reverse_copy(std::rbegin(word), first_nonmax_partition, std::back_inserter(res));
	// std::begin(word) != first_nonmax_partition
	if (std::prev(std::rend(word)) != first_nonmax_partition) {
		// Process the first Abs_i. If its region is even, the fractional part is 0. As we increment
		// the clocks by a value (0,1), the fractional part will be >0, and thus we need to go to the
		// next region. Otherwise, the clocks stay in the same region.
		res.push_back(std::set<ABRegionSymbol<Location, ActionType>>{});
		// TODO this condition is wrong, we need to check every element separately
		if (get_region_index(*(word.begin()->begin())) % 2 == 0) {
			// Region is even, we need to increment.
			std::transform(word.begin()->begin(),
			               word.begin()->end(),
			               std::inserter(res.back(), res.back().end()),
			               increment_region_index);
		} else {
			// Region is odd, no need to increment.
			std::copy(word.begin()->begin(),
			          word.begin()->end(),
			          std::inserter(res.back(), res.back().end()));
		}
		// Copy all other abs_i which are not the first nor the last. Those never change.
		std::reverse_copy(std::next(first_nonmax_partition),
		                  std::prev(word.rend()),
		                  std::back_inserter(res));
	}
	return res;
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
