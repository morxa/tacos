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
#include "model/types.h"

#include <spdlog/spdlog.h>

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

GologProgram::GologProgram(const std::string           &program,
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

	populate_fluents(relevant_fluent_symbols);
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
	GologLocation location{get_all_satisfied_fluents(*get_empty_history()),
	                       std::make_shared<gologpp::ManagedTerm>(main->semantics().plterm()),
	                       std::make_shared<gologpp::History>()};
	location.history->attach_semantics(*semantics);
	return location;
}

GologConfiguration
GologProgram::get_initial_configuration() const
{
	return GologConfiguration{get_initial_location(),
	                          {std::make_pair(std::string{"golog"}, tacos::Clock{})}};
}

bool
GologProgram::is_accepting_configuration(const GologConfiguration &configuration) const
{
	return gologpp::is_final(*configuration.location.remaining_program,
	                         *configuration.location.history);
}

std::set<std::string>
GologProgram::get_satisfied_fluents(
  const gologpp::History                                             &history,
  const std::map<std::string, gologpp::Reference<gologpp::Fluent> *> &fluents) const
{
	std::set<std::string> satisfied_fluents;
	for (const auto &[name, fluent] : fluents) {
		if (static_cast<bool>(fluent->semantics().evaluate({}, history))) {
			satisfied_fluents.insert(name);
		}
	}
	return satisfied_fluents;
}

std::set<std::string>
GologProgram::get_relevant_satisfied_fluents(const gologpp::History &history) const
{
	return get_satisfied_fluents(history, relevant_fluents);
}

std::set<std::string>
GologProgram::get_all_satisfied_fluents(const gologpp::History &history) const
{
	return get_satisfied_fluents(history, all_fluents);
}

void
GologProgram::populate_fluents(const std::set<std::string> &relevant_fluent_symbols)
{
	for (const auto &global : gologpp::global_scope().globals()) {
		const auto fluent = gologpp::global_scope().lookup_global<gologpp::Fluent>(global->name());
		if (fluent) {
			if (!fluent->type().is<gologpp::BoolType>()) {
				SPDLOG_WARN("Fluent {} is not of type bool, ignoring! Make sure this fluent is irrelevant.",
				            fluent->name());
				continue;
			}
			std::vector<std::vector<gologpp::Expression *>> possible_arg_vectors = {{}};
			for (auto i = 0; i < fluent->arity(); i++) {
				std::vector<std::vector<gologpp::Expression *>> new_possible_arg_vectors;
				for (const auto &[_, domain] : *gologpp::global_scope().get_domains()) {
					if (fluent->parameter(i)->type().name() == domain->type().name()) {
						for (const auto &arg : domain->elements()) {
							for (const auto &args : possible_arg_vectors) {
								auto new_args = args;
								new_args.push_back(arg.get());
								new_possible_arg_vectors.push_back(new_args);
							}
						}
					}
				}
				possible_arg_vectors = new_possible_arg_vectors;
			}
			for (const auto &args : possible_arg_vectors) {
				auto ref = fluent->make_ref(args);
				ref->attach_semantics(*semantics);
				SPDLOG_DEBUG("Tracking fluent: {}", ref->str());
				all_fluents.emplace(ref->str(), ref);
				if (relevant_fluent_symbols.find(ref->str()) != std::end(relevant_fluent_symbols)) {
					SPDLOG_DEBUG("Tracking relevant fluent: {}", ref->str());
					relevant_fluents.emplace(ref->str(), ref);
				}
			}
		}
	}
}

} // namespace tacos::search
