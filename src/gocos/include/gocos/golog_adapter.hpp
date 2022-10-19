/***************************************************************************
 *  golog_adapter.hpp - Generate successors of Golog configurations
 *
 *  Created:   Tue  7 Jun 16:33:55 CEST 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include "golog_adapter.h"
#include "utilities/types.h"

#include <execution/activity.h>
#include <model/action.h>
#include <model/gologpp.h>
#include <model/types.h>
#include <semantics/readylog/execution.h>
#include <semantics/readylog/history.h>
#include <semantics/readylog/utilities.h>

#include <algorithm>
#include <iterator>
#include <variant>

namespace tacos::search {

template <bool use_location_constraints, bool use_set_semantics>
std::multimap<std::string, CanonicalABWord<GologLocation, std::string>>
get_next_canonical_words<GologProgram,
                         std::string,
                         std::string,
                         use_location_constraints,
                         use_set_semantics>::
operator()(
  const GologProgram                                                                     &program,
  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<std::string>,
                                                 logic::AtomicProposition<ATAInputType>> &ata,
  const std::pair<GologConfiguration, ATAConfiguration<std::string>> &ab_configuration,
  [[maybe_unused]] const RegionIndex                                  increment,
  const RegionIndex                                                   K)
{
	std::multimap<std::string, CanonicalABWord<GologLocation, std::string>> successors;
	const auto &[_, remaining_program, history] = ab_configuration.first.location;
	auto golog_successors =
	  program.get_semantics().trans_all(*history,
	                                    remaining_program.get(),
	                                    details::get_clock_values(
	                                      ab_configuration.first.clock_valuations));
	for (const auto &golog_successor : golog_successors) {
		// const auto &[plan, program_suffix, new_history] = golog_successor;
		const auto       &plan             = std::get<0>(golog_successor);
		const auto       &program_suffix   = std::get<1>(golog_successor);
		const auto       &new_history      = std::get<2>(golog_successor);
		const std::string action           = plan->elements().front().instruction().str();
		auto              clock_valuations = ab_configuration.first.clock_valuations;
		if (program.has_action_clock(action)) {
			std::string prim_action;
			if (action.substr(0, 6) == "start(") {
				prim_action = action.substr(6, action.size() - 1 - 6);
			} else {
				assert(action.substr(0, 4) == "end(");
				prim_action = action.substr(4, action.size() - 1 - 4);
			}
			// Reset to the clock to 0 if it already exists and otherwise insert a new clock.
			clock_valuations[prim_action].reset();
		}
		if (std::find_if(clock_valuations.begin(),
		                 clock_valuations.end(),
		                 [](const auto &valuation) { return valuation.first != "golog"; })
		    != std::end(clock_valuations)) {
			clock_valuations.erase("golog");
		} else {
			clock_valuations["golog"] = 0;
		}
		const auto ata_successors = [&]() {
			if constexpr (use_location_constraints) {
				const auto relevant_satisfied_fluents = [&]() {
					auto fluents = program.get_relevant_satisfied_fluents(*new_history);
					// Add the currently occuring action as additional fluent of the form 'occ(action)'.
					const std::string occ_fluent = fmt::format("occ({})", action);
					if (program.is_relevant_fluent(occ_fluent)) {
						fluents.insert(occ_fluent);
					}
					return fluents;
				}();
				return ata.make_symbol_step(ab_configuration.second, relevant_satisfied_fluents);
			} else {
				return ata.make_symbol_step(ab_configuration.second, action);
			}
		}();
		for (const auto &ata_successor : ata_successors) {
			[[maybe_unused]] auto successor = successors.insert(std::make_pair(
			  action,
			  get_canonical_word(GologConfiguration{{program.get_all_satisfied_fluents(*new_history),
			                                         program_suffix,
			                                         new_history},
			                                        clock_valuations},
			                     ata_successor,
			                     K)));
			SPDLOG_TRACE("{}, {}): Getting {} with symbol {}",
			             ab_configuration.first,
			             ab_configuration.second,
			             successor->second,
			             action);
		}
	}
	return successors;
}

} // namespace tacos::search
