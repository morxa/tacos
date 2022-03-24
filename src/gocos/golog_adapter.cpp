/***************************************************************************
 *  golog_adapter.cpp - Generate successors of Golog configurations
 *
 *  Created:   Fri 24 Sep 16:32:37 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include "gocos/golog_adapter.h"

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

std::ostream &
operator<<(std::ostream &os, const GologLocation &location)
{
	os << "(";
	if (location.remaining_program) {
		os << gologpp::ReadylogContext::instance().to_string(*location.remaining_program);
	} else {
		os << "[]";
	}
	os << ", ";
	os << location.history->special_semantics() << ")";
	return os;
}

bool
operator<(const GologLocation &first, const GologLocation &second)
{
	if (*first.remaining_program < *second.remaining_program) {
		return true;
	}
	if (*second.remaining_program < *first.remaining_program) {
		return false;
	}
	return first.history->special_semantics().get_managed_term()
	       < second.history->special_semantics().get_managed_term();
}

std::multimap<std::string, CanonicalABWord<GologLocation, std::string>>
get_next_canonical_words<GologProgram, std::string, std::string, false>::operator()(
  const GologProgram &                                                                   program,
  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<std::string>,
                                                 logic::AtomicProposition<std::string>> &ata,
  const std::pair<GologConfiguration, ATAConfiguration<std::string>> &ab_configuration,
  const RegionIndex                                                   increment,
  const RegionIndex                                                   K)
{
	std::multimap<std::string, CanonicalABWord<GologLocation, std::string>> successors;
	const auto &[remaining_program, history] = ab_configuration.first.location;
	auto golog_successors = program.get_semantics().trans_all(*history, remaining_program.get());
	// Add terminate actions if we are at the max region index.
	if (increment == 2 * K + 1) {
		// Only add the ctl terminate action if there is at least one env action.
		if (std::any_of(begin(golog_successors), end(golog_successors), [this](const auto &successor) {
			    const std::string action = std::get<0>(successor)->elements().front().instruction().str();
			    return environment_actions.find(action) != std::end(environment_actions);
		    })) {
			const std::string action         = "ctl_terminate";
			const auto        ata_successors = ata.make_symbol_step(ab_configuration.second, action);
			for (auto &ata_successor : ata_successors) {
				auto clock_valuations = ab_configuration.first.clock_valuations;
				successors.insert(std::make_pair(
				  action,
				  get_canonical_word(GologConfiguration{{program.get_empty_program(), history},
				                                        clock_valuations},
				                     ata_successor,
				                     K)));
			}
		}
		// Only add the env terminate action if there is at least one ctl action.
		if (std::any_of(begin(golog_successors), end(golog_successors), [this](const auto &successor) {
			    const std::string action = std::get<0>(successor)->elements().front().instruction().str();
			    return controller_actions.find(action) != std::end(controller_actions);
		    })) {
			const std::string action         = "env_terminate";
			const auto        ata_successors = ata.make_symbol_step(ab_configuration.second, action);
			for (auto &ata_successor : ata_successors) {
				auto clock_valuations = ab_configuration.first.clock_valuations;
				successors.insert(std::make_pair(
				  action,
				  get_canonical_word(GologConfiguration{{program.get_empty_program(), history},
				                                        clock_valuations},
				                     ata_successor,
				                     K)));
			}
		}
	}
	for (const auto &golog_successor : golog_successors) {
		const auto &[plan, program_suffix, new_history] = golog_successor;
		std::string action                              = plan->elements().front().instruction().str();
		const auto  ata_successors = ata.make_symbol_step(ab_configuration.second, action);
		for (const auto &ata_successor : ata_successors) {
			auto clock_valuations           = ab_configuration.first.clock_valuations;
			clock_valuations["golog"]       = 0;
			[[maybe_unused]] auto successor = successors.insert(std::make_pair(
			  action,
			  get_canonical_word(GologConfiguration{{program_suffix, new_history}, clock_valuations},
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
