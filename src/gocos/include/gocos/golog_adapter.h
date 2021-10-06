/***************************************************************************
 *  golog_adapter.h - Generate successors of Golog configurations
 *
 *  Created:   Tue 21 Sep 13:47:24 CEST 2021
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

#include "NamedType/underlying_functionalities.hpp"
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

/** The location of a golog program.
 * This represents the current state of a program execution and consists of a gologpp term for the
 * remaining program, as well as a gologpp history.
 */
struct GologLocation
{
	/** The program yet to be executed. */
	gologpp::shared_ptr<gologpp::ManagedTerm> remaining_program;
	/** A history of already executed actions. */
	gologpp::shared_ptr<gologpp::History> history;
};

/** Print a golog location to an ostream. */
std::ostream &operator<<(std::ostream &os, const GologLocation &);
/** Compare two golog locations. */
bool operator<(const GologLocation &, const GologLocation &);

/** A configuration of a Golog program.
 * Similar to TAs, a configuration is a program location with a set of clock valuations. */
using GologConfiguration = tacos::PlantConfiguration<GologLocation>;
/** An expanded state (location, clock_name, clock_valuation) of a Golog program. */
using GologState = PlantState<GologLocation>;

/** @brief Compute all successors for one particular time successor and all possible symbols.
 * Compute the successors by following all transitions in the program and ATA for one time successor
 * and all actions that can be executed in the program.
 */
std::multimap<std::string, CanonicalABWord<GologLocation, std::string>> get_next_canonical_words(
  gologpp::Semantics<gologpp::Instruction> &                                             program,
  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<std::string>,
                                                 logic::AtomicProposition<std::string>> &ata,
  const std::pair<GologConfiguration, ATAConfiguration<std::string>> &ab_configuration,
  RegionIndex                                                         K);

} // namespace tacos::search
