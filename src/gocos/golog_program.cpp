/***************************************************************************
 *  golog_program.h - Golog++ Program Wrapper
 *
 *
 *  Created:   Tue 19 Oct 15:53:30 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include "gocos/golog_program.h"

#include "gocos/golog_symbols.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <execution/history.h>
#include <model/expressions.h>
#include <parser/parser.h>
#include <semantics/readylog/utilities.h>
#pragma GCC diagnostic pop

#include <fmt/format.h>

namespace tacos::search {

bool GologProgram::initialized = false;

GologProgram::GologProgram(const std::string &          program,
                           const std::set<std::string> &relevant_fluent_symbols)
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
		teardown();
		throw std::invalid_argument("Golog program does not contain a main procedure");
	}
	main      = procedure->ref({});
	semantics = &gologpp::ReadylogContext::instance().semantics_factory();
	main->attach_semantics(*semantics);
	empty_history.reset(new gologpp::History());
	empty_history->attach_semantics(*semantics);
	empty_program.reset(new gologpp::ManagedTerm(gologpp::make_ec_list({})));
	gologpp::global_scope().implement_globals(*semantics, gologpp::ReadylogContext::instance());

	populate_relevant_fluents(relevant_fluent_symbols);
}

GologProgram::~GologProgram()
{
	teardown();
}

void
GologProgram::teardown()
{
	empty_program.reset();
	empty_history.reset();
	gologpp::global_scope().clear();
	gologpp::ReadylogContext::shutdown();
	initialized = false;
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

std::set<std::string>
GologProgram::get_satisfied_fluents(const gologpp::History &history) const
{
	std::set<std::string> satisfied_fluents;
	for (const auto &global : relevant_fluents) {
		if (static_cast<bool>(global->semantics().evaluate({}, history))) {
			satisfied_fluents.insert(global->to_string(""));
		}
	}
	return satisfied_fluents;
}

void
GologProgram::populate_relevant_fluents(const std::set<std::string> &relevant_fluent_symbols)
{
	for (const auto &fluent_symbol : relevant_fluent_symbols) {
		const auto [name, args] = split_symbol(fluent_symbol);
		const auto fluent       = gologpp::global_scope().lookup_global<gologpp::Fluent>(name);
		if (!fluent) {
			throw std::invalid_argument(fmt::format("Fluent {} is not known in the Golog program", name));
		}
		std::vector<gologpp::Expression *> params;
		for (const auto &arg : args) {
			params.push_back(gologpp::global_scope().get_symbol(arg));
		}
		auto ref = fluent->make_ref(params);
		ref->attach_semantics(*semantics);
		relevant_fluents.emplace(ref);
	}
}

} // namespace tacos::search
