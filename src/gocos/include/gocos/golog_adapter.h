/***************************************************************************
 *  golog_adapter.h - Generate successors of Golog configurations
 *
 *  Created:   Tue 21 Sep 13:47:24 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

#include "golog_program.h"
#include "search/adapter.h"
#include "search/canonical_word.h"

#include <spdlog/spdlog.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <execution/plan.h>
#include <model/gologpp.h>
#include <semantics/readylog/semantics.h>
#include <semantics/readylog/utilities.h>
#pragma GCC diagnostic pop

namespace tacos::search {
/** An expanded state (location, clock_name, clock_valuation) of a Golog program. */
using GologState = PlantState<GologLocation>;

/** @brief Compute all successors for one particular time successor and all possible symbols.
 * Compute the successors by following all transitions in the program and ATA for one time successor
 * and all actions that can be executed in the program.
 */
template <>
class get_next_canonical_words<GologProgram, std::string, std::string, false>
{
public:
	/** Construct the comparator with the given action partitioning.
	 * @param controller_actions The actions that the controller can do
	 * @param environment_actions The actions that the environment can do
	 */
	get_next_canonical_words(const std::set<std::string> &controller_actions,
	                         const std::set<std::string> &environment_actions)
	: controller_actions(controller_actions), environment_actions(environment_actions)
	{
	}

	std::multimap<std::string, CanonicalABWord<GologLocation, std::string>> operator()(
	  const GologProgram &                                                                   program,
	  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<std::string>,
	                                                 logic::AtomicProposition<std::string>> &ata,
	  const std::pair<GologConfiguration, ATAConfiguration<std::string>> &ab_configuration,
	  const RegionIndex                                                   increment,
	  const RegionIndex                                                   K);

private:
	std::set<std::string> controller_actions;
	std::set<std::string> environment_actions;
};

/** Print a golog location to an ostream. */
std::ostream &operator<<(std::ostream &os, const GologLocation &);

} // namespace tacos::search
