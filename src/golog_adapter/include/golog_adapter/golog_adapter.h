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
#include "utilities/type_traits.h"

#include <fmt/ostream.h>
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

/** @brief Compute all Golog successors for one particular time successor and all possible symbols.
 *
 * Compute the successors by following all transitions in the program and ATA for one time successor
 * and all actions that can be executed in the program.
 */
template <bool use_location_constraints, bool use_set_semantics>
class get_next_canonical_words<GologProgram,
                               std::string,
                               std::string,
                               use_location_constraints,
                               use_set_semantics>
{
public:
	/** The type of the input symbols that the ATA accepts. */
	using ATAInputType =
	  /* if use_set_semantics is true, then the ATASymbolT is the same as
	   * std::set<ConstraintSymbolType>, otherwise it is just ConstraintSymbolType> */
	  typename std::disjunction<
	    utilities::values_equal<use_set_semantics, false, std::string>,
	    utilities::values_equal<use_set_semantics, true, std::set<std::string>>,
	    std::void_t<void>>::type;
	/** Construct the comparator with the given action partitioning.
	 * @param controller_actions The actions that the controller can do
	 * @param environment_actions The actions that the environment can do
	 */
	get_next_canonical_words(const std::set<std::string> &controller_actions,
	                         const std::set<std::string> &environment_actions)
	: controller_actions(controller_actions), environment_actions(environment_actions)
	{
		static_assert(!use_location_constraints || (use_location_constraints && use_set_semantics));
	}

	/** @brief Compute all successors for one particular time successor and all possible symbols.
	 * Compute the successors by following all transitions in the program and ATA for one time
	 * successor and all actions that can be executed in the program.
	 * @param program The Golog program
	 * @param ata The ATA for the specification
	 * @param ab_configuration The current configuration of the program and the ATA
	 * @param increment The current time increment
	 * @param K The maximal constant occurring in program or ATA
	 */
	std::multimap<std::string, CanonicalABWord<GologLocation, std::string>> operator()(
	  const GologProgram                                                                     &program,
	  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<std::string>,
	                                                 logic::AtomicProposition<ATAInputType>> &ata,
	  const std::pair<GologConfiguration, ATAConfiguration<std::string>> &ab_configuration,
	  const RegionIndex                                                   increment,
	  const RegionIndex                                                   K);

private:
	std::set<std::string> controller_actions;
	std::set<std::string> environment_actions;
};

/** Print a Golog location to an ostream. */
std::ostream &operator<<(std::ostream &os, const GologLocation &);

namespace details {
std::map<std::string, double> get_clock_values(const ClockSetValuation &clock_valuations);
} // namespace details

} // namespace tacos::search

namespace fmt {

template <>
struct formatter<tacos::search::GologLocation> : ostream_formatter
{
};

} // namespace fmt

#include "golog_adapter.hpp"
