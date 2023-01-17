/***************************************************************************
 *  golog_program.h - Golog++ Program Wrapper
 *
 *  Created:   Tue 19 Oct 15:48:45 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include <memory>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <execution/plan.h>
#include <model/gologpp.h>
#include <model/reference.h>
#include <semantics/readylog/reference.h>
#include <semantics/readylog/semantics.h>
#include <semantics/readylog/utilities.h>
#include <utilities/types.h>
#pragma GCC diagnostic pop

namespace tacos::search {

class GologProgram;

/** @brief The location of a Golog program.
 *
 * This represents the current state of a program execution and consists of a gologpp term for the
 * remaining program, as well as a gologpp history.
 */
struct GologLocation
{
	/** The currently satisfied fluents. */
	std::set<std::string> satisfied_fluents;
	/** The program yet to be executed. */
	gologpp::shared_ptr<gologpp::ManagedTerm> remaining_program;
	/** A history of already executed actions. */
	gologpp::shared_ptr<gologpp::History> history;
};

/** @brief A configuration of a Golog program.
 *
 * Similar to TAs, a configuration is a program location with a set of clock valuations. */
using GologConfiguration = tacos::PlantConfiguration<GologLocation>;

/** Compare two Golog locations. */
bool operator<(const GologLocation &, const GologLocation &);

/** @brief Wrapper for a Golog++ program.
 *
 * This class manages a Golog++ program and provides additional functionality
 * needed for synthesizing a controller against this program. */
class GologProgram
{
public:
	/** The underlying location type.
	 * @see GologLocatoin
	 */
	using Location = GologLocation;
	/** Construct a program from a program string.
	 * @param program A Golog program as string.
	 * @param relevant_fluent_symbols The fluent symbols that are relevant for the specification,
	 * i.e., the fluents mentioned in the specification.
	 */
	GologProgram(const std::string           &program,
	             const std::set<std::string> &relevant_fluent_symbols = {},
	             bool                         all_action_clocks       = true,
	             std::set<std::string>        action_clock_names      = {});

	/** Clean up the Golog program and release global resources. */
	~GologProgram();

	/** Get the initial location of the program.
	 * @see GologLocation
	 */
	GologLocation get_initial_location() const;
	/** Get the initial configuration of the program.
	 * A configuration consists of a location and clock valuations. For a Golog program, there is only
	 * a single clock called 'golog'.
	 * @see GologConfiguration
	 */
	GologConfiguration get_initial_configuration() const;

	/** Get the underlying golog++ semantics object for the program. */
	gologpp::Semantics<gologpp::Instruction> &
	get_semantics() const
	{
		return main->semantics();
	}

	/** Get a pointer to the empty history. */
	gologpp::shared_ptr<gologpp::History>
	get_empty_history() const
	{
		return empty_history;
	}

	/** Get a pointer to the empty program. */
	gologpp::shared_ptr<gologpp::ManagedTerm>
	get_empty_program() const
	{
		return empty_program;
	}

	/** Check if a program is accepting, i.e., terminates, in the given configuration. */
	bool is_accepting_configuration(const GologConfiguration &configuration) const;

	/** Get the satisfied relevant fluents at the point of the given history. */
	std::set<std::string> get_relevant_satisfied_fluents(const gologpp::History &history) const;

	/** Get the satisfied relevant fluents at the point of the given history. */
	std::set<std::string> get_all_satisfied_fluents(const gologpp::History &history) const;

	/** Check if a given fluent is relevant. */
	bool
	is_relevant_fluent(const std::string &fluent) const
	{
		return relevant_fluents.find(fluent) != relevant_fluents.end();
	}

	/** Check if action clocks should be used. */
	bool
	has_action_clock(const std::string &action) const
	{
		return (all_action_clocks && action.substr(0, 6) == "start(")
		       || action_clock_names.find(action) != action_clock_names.end();
	}

private:
	void                  teardown();
	void                  populate_fluents(const std::set<std::string> &relevant_fluent_symbols);
	std::set<std::string> get_satisfied_fluents(
	  const gologpp::History                                             &history,
	  const std::map<std::string, gologpp::Reference<gologpp::Fluent> *> &fluents) const;

	// We can only have one program at a time, because the program accesses the global scope. Thus,
	// make sure that we do not run two programs simultaneously.
	static bool                                                  initialized;
	std::shared_ptr<gologpp::Procedure>                          procedure;
	gologpp::Instruction                                        *main;
	gologpp::SemanticsFactory                                   *semantics;
	std::shared_ptr<gologpp::History>                            empty_history;
	std::shared_ptr<gologpp::ManagedTerm>                        empty_program;
	std::map<std::string, gologpp::Reference<gologpp::Fluent> *> all_fluents;
	std::map<std::string, gologpp::Reference<gologpp::Fluent> *> relevant_fluents;
	bool                                                         all_action_clocks;
	std::set<std::string>                                        action_clock_names;
};

} // namespace tacos::search
