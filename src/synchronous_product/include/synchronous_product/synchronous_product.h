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

#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <variant>

/** Get the regionalized synchronous product of a TA and an ATA. */
namespace synchronous_product {

using automata::ClockValuation;
using automata::Time;
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
using TARegionState = std::tuple<Location, std::string, std::size_t>;
/** An ATARegionState is a pair (formula, clock_region) */
template <typename ActionType>
using ATARegionState = std::pair<logic::MTLFormula<ActionType>, std::size_t>;
/** An ABRegionSymbol is either a TARegionState or an ATARegionState */
template <typename Location, typename ActionType>
using ABRegionSymbol = std::variant<TARegionState<Location>, ATARegionState<ActionType>>;

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
std::vector<std::set<ABRegionSymbol<Location, ActionType>>>
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
	automata::ta::TimedAutomatonRegions                         regionSet{K};
	std::vector<std::set<ABRegionSymbol<Location, ActionType>>> abs;
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
