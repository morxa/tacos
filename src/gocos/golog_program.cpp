/***************************************************************************
 *  golog_program.h - Golog++ Program Wrapper
 *
 *
 *  Created:   Tue 19 Oct 15:53:30 CEST 2021
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

#include "gocos/golog_program.h"

#include <execution/history.h>
#include <parser/parser.h>
#include <semantics/readylog/utilities.h>

#include <stdexcept>

namespace tacos::search {

bool GologProgram::initialized = false;

GologProgram::GologProgram(const std::string &program)
{
	if (initialized) {
		throw std::runtime_error("Golog environment has already been initialized!");
	}
	initialized = true;
	gologpp::eclipse_opts options;
	options.trace    = false;
	options.toplevel = false;
	options.guitrace = true;
	gologpp::ReadylogContext::init(options);
	gologpp::parser::parse_string(program);
	procedure = gologpp::global_scope().lookup_global<gologpp::Procedure>("main");
	if (procedure == nullptr) {
		throw std::invalid_argument("Golog program does not contain a main procedure");
	}
	main      = procedure->ref({});
	semantics = &gologpp::ReadylogContext::instance().semantics_factory();
	main->attach_semantics(*semantics);
	empty_history.reset(new gologpp::History());
	empty_history->attach_semantics(*semantics);
	empty_program.reset(new gologpp::ManagedTerm(gologpp::make_ec_list({})));
	gologpp::global_scope().implement_globals(*semantics, gologpp::ReadylogContext::instance());
}

GologLocation
GologProgram::get_initial_location() const
{
	GologLocation location;
	location.remaining_program = std::make_shared<gologpp::ManagedTerm>(main->semantics().plterm());
	location.history.reset(new gologpp::History());
	location.history->attach_semantics(*semantics);
	return location;
}

GologConfiguration
GologProgram::get_initial_configuration() const
{
	GologConfiguration configuration;
	configuration.location = get_initial_location();
	configuration.clock_valuations.insert(std::make_pair(std::string{"golog"}, tacos::Clock{}));
	return configuration;
}

bool
GologProgram::is_accepting_configuration(const GologConfiguration &configuration) const
{
	return gologpp::is_final(*configuration.location.remaining_program,
	                         *configuration.location.history);
}

GologProgram::~GologProgram()
{
	empty_program.reset();
	empty_history.reset();
	gologpp::global_scope().clear();
	gologpp::ReadylogContext::shutdown();
	initialized = false;
}

} // namespace tacos::search
