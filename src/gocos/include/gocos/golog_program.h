/***************************************************************************
 *  golog_program.h - Golog++ Program Wrapper
 *
 *  Created:   Tue 19 Oct 15:48:45 CEST 2021
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

#include "utilities/types.h"

#include <memory>
#include <string>

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

/** A configuration of a Golog program.
 * Similar to TAs, a configuration is a program location with a set of clock valuations. */
using GologConfiguration = tacos::PlantConfiguration<GologLocation>;

/** Compare two golog locations. */
bool operator<(const GologLocation &, const GologLocation &);

/** Wrapper for a Golog++ program.
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
	 * @param program A golog program as string.
	 */
	GologProgram(const std::string &program);

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

private:
	void teardown();

	// We can only have one program at a time, because the program accesses the global scope. Thus,
	// make sure that we do not run two programs simultaneously.
	static bool                           initialized;
	std::shared_ptr<gologpp::Procedure>   procedure;
	gologpp::Instruction *                main;
	gologpp::SemanticsFactory *           semantics;
	std::shared_ptr<gologpp::History>     empty_history;
	std::shared_ptr<gologpp::ManagedTerm> empty_program;
};

} // namespace tacos::search
