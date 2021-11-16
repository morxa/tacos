/***************************************************************************
 *  golog_adapter.cpp - Generate successors of Golog configurations
 *
 *  Created:   Fri 24 Sep 16:32:37 CEST 2021
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

#include "gocos/golog_adapter.h"

#include <semantics/readylog/execution.h>
#include <semantics/readylog/history.h>

namespace tacos::search {

std::ostream &
operator<<(std::ostream &os, const GologLocation &location)
{
	os << "(" << gologpp::ReadylogContext::instance().to_string(*location.remaining_program) << ", "
	   << location.history->special_semantics() << ")";
	return os;
}

std::ostream &
operator<<(std::ostream &os, const GologConfiguration &)
{
	return os;
}

bool
operator<(const GologLocation &first, const GologLocation &second)
{
	// TODO we should not just compare the string representations
	std::stringstream s1, s2;
	s1 << first;
	s2 << second;
	return s1.str() < s2.str();
}

std::multimap<std::string, CanonicalABWord<GologLocation, std::string>>
get_next_canonical_words<GologProgram, std::string, std::string, false>::operator()(
  const GologProgram &                                                                   program,
  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<std::string>,
                                                 logic::AtomicProposition<std::string>> &ata,
  const std::pair<GologConfiguration, ATAConfiguration<std::string>> &ab_configuration,
  const RegionIndex,
  const RegionIndex K)
{
	std::multimap<std::string, CanonicalABWord<GologLocation, std::string>> successors;
	const auto &[remaining_program, history] = ab_configuration.first.location;
	const auto golog_successors =
	  program.get_semantics().trans_all(*history, remaining_program.get());
	for (const auto &golog_successor : golog_successors) {
		const auto &[plan, program_suffix, new_history] = golog_successor;
		std::string action                              = plan->elements().front().instruction().str();
		const auto  ata_successors = ata.make_symbol_step(ab_configuration.second, action);
		for (const auto &ata_successor : ata_successors) {
			[[maybe_unused]] auto successor = successors.insert(std::make_pair(
			  action,
			  get_canonical_word(GologConfiguration{{program_suffix, new_history},
			                                        ab_configuration.first.clock_valuations},
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
