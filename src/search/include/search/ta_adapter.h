/***************************************************************************
 *  ta_adapter.h - Generate successors of TA configurations
 *
 *  Created:   Thu  9 Sep 09:52:44 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

#include "adapter.h"
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
 * This is a partial template specialization of the generic plant adapter.
 * */
template <typename LocationT,
          typename ActionType,
          typename ConstraintSymbolType,
          bool use_location_constraints>
class get_next_canonical_words<automata::ta::TimedAutomaton<LocationT, ActionType>,
                               ActionType,
                               ConstraintSymbolType,
                               use_location_constraints>
{
public:
	get_next_canonical_words(const std::set<ActionType> & = {}, const std::set<ActionType> & = {})
	{
	}
	/** Get the next canonical words. */
	std::multimap<
	  ActionType,
	  CanonicalABWord<typename automata::ta::TimedAutomaton<LocationT, ActionType>::Location,
	                  ConstraintSymbolType>>
	operator()(
	  const automata::ta::TimedAutomaton<LocationT, ActionType> &ta,
	  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ConstraintSymbolType>,
	                                                 logic::AtomicProposition<ConstraintSymbolType>>
	    &                                                      ata,
	  const std::pair<typename automata::ta::TimedAutomaton<LocationT, ActionType>::Configuration,
	                  ATAConfiguration<ConstraintSymbolType>> &ab_configuration,
	  const RegionIndex,
	  const RegionIndex K)
	{
		static_assert(use_location_constraints || std::is_same_v<ActionType, ConstraintSymbolType>);
		static_assert(
		  !use_location_constraints
		  || std::is_same_v<typename automata::ta::TimedAutomaton<LocationT, ActionType>::Location,
		                    ConstraintSymbolType>);
		std::multimap<
		  ActionType,
		  CanonicalABWord<typename automata::ta::TimedAutomaton<LocationT, ActionType>::Location,
		                  ConstraintSymbolType>>
		  successors;
		for (const auto &symbol : ta.get_alphabet()) {
			SPDLOG_TRACE("({}, {}): Symbol {}", ab_configuration.first, ab_configuration.second, symbol);
			const std::set<typename automata::ta::TimedAutomaton<LocationT, ActionType>::Configuration>
			  ta_successors = ta.make_symbol_step(ab_configuration.first, symbol);
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
};

} // namespace tacos::search
