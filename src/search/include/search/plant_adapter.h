/***************************************************************************
 *  plant_adapter.h - Wrap the underlying execution model (e.g., a TA)
 *
 *  Created:   Thu  9 Sep 09:52:44 CEST 2021
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

#include "canonical_word.h"
#include "utilities/types.h"

#include <spdlog/spdlog.h>

#include <vector>

namespace tacos::search {

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
		std::transform(
		  g_i.begin(),
		  g_i.end(),
		  std::inserter(abs_i, abs_i.end()),
		  [&](const ABSymbol<Location, ConstraintSymbolType> &w)
		    -> ABRegionSymbol<Location, ConstraintSymbolType> {
			  if (std::holds_alternative<TAState<Location>>(w)) {
				  const TAState<Location> &s = std::get<TAState<Location>>(w);
				  return PlantRegionState<Location>{s.location,
				                                    s.clock,
				                                    regionSet.getRegionIndex(s.clock_valuation)};
			  } else {
				  const ATAState<ConstraintSymbolType> &s = std::get<ATAState<ConstraintSymbolType>>(w);
				  return ATARegionState<ConstraintSymbolType>{s.location,
				                                              regionSet.getRegionIndex(s.clock_valuation)};
			  }
		  });
		abs.push_back(abs_i);
	}
	assert(is_valid_canonical_word(abs));
	return abs;
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
