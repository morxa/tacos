/***************************************************************************
 *  ta_adapter.h - Generate successors of TA configurations
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

/** Short-hand type alias for a configuration of a TA */
template <typename LocationT>
using TAConfiguration = automata::ta::TAConfiguration<LocationT>;

/** An expanded state (location, clock_name, clock_valuation) of a TA. */
template <typename LocationT>
using TAState = PlantState<automata::ta::Location<LocationT>>;

/** @brief Compute all successors for one particular time successor.
 * Compute the successors by following all transitions in the TA and ATA for one time successor
 * and all possible symbols.
 * */
template <typename Location /* automata::ta::Location<std::string> */,
          typename ActionType,
          typename ConstraintSymbolType,
          bool use_location_constraints = false>
std::multimap<ActionType, CanonicalABWord<Location, ConstraintSymbolType>>
get_next_canonical_words(
  const automata::ta::TimedAutomaton<typename Location::UnderlyingType, ActionType> &ta,
  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ConstraintSymbolType>,
                                                 logic::AtomicProposition<ConstraintSymbolType>>
    &                                                      ata,
  const std::pair<TAConfiguration<typename Location::UnderlyingType>,
                  ATAConfiguration<ConstraintSymbolType>> &ab_configuration,
  RegionIndex                                              K)
{
	static_assert(use_location_constraints || std::is_same_v<ActionType, ConstraintSymbolType>);
	static_assert(!use_location_constraints || std::is_same_v<Location, ConstraintSymbolType>);
	std::multimap<ActionType, CanonicalABWord<Location, ConstraintSymbolType>> successors;
	for (const auto &symbol : ta.get_alphabet()) {
		SPDLOG_TRACE("({}, {}): Symbol {}", ab_configuration.first, ab_configuration.second, symbol);
		const std::set<TAConfiguration<typename Location::UnderlyingType>> ta_successors =
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
				                                      logic::AtomicProposition{ta_successor.location});
			}
			for (const auto &ata_successor : ata_successors) {
				SPDLOG_TRACE("({}, {}): ATA successor {}",
				             ab_configuration.first,
				             ab_configuration.second,
				             ata_successor);
				[[maybe_unused]] auto successor = successors.insert(
				  std::make_pair(symbol, get_canonical_word(ta_successor, ata_successor, K)));
				SPDLOG_TRACE("({}, {}): Getting {} with symbol {}",
				             ab_configuration.first,
				             ab_configuration.second,
				             successor->second,
				             symbol);
			}
		}
	}
	return successors;
}

} // namespace tacos::search
