/***************************************************************************
 *  test_golog_words.cpp - Test successor word generation for Golog
 *
 *  Created:   Tue 21 Sep 13:52:54 CEST 2021
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

#include "golog_test_fixture.h"

#include "gocos/golog_adapter.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "parser/parser.h"
#include "semantics/readylog/execution.h"

#include <semantics/readylog/history.h>
#include <semantics/readylog/utilities.h>

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <variant>

using gologpp::parser::parse_string;

using tacos::search::get_next_canonical_words;
using namespace tacos::logic;

using CanonicalABWord = tacos::search::CanonicalABWord<tacos::search::GologLocation, std::string>;

GologTestFixture::GologTestFixture()
{
	gologpp::eclipse_opts options;
	options.trace    = false;
	options.toplevel = false;
	options.guitrace = true;
	gologpp::ReadylogContext::init(options);
	semantics = &gologpp::ReadylogContext::instance().semantics_factory();
	history.reset(new gologpp::History());
}

GologTestFixture::~GologTestFixture()
{
	history.reset();
	gologpp::global_scope().clear();
	gologpp::ReadylogContext::shutdown();
}

void
GologTestFixture::init_program(const std::string &program)
{
	if (initialized) {
		throw std::logic_error("Cannot reinitialize the program");
	}
	initialized = true;
	parse_string(program);
	main_proc = gologpp::global_scope().lookup_global<gologpp::Procedure>("main");
	REQUIRE(main_proc != nullptr);
	main = main_proc->ref({});
	main->attach_semantics(*semantics);
	gologpp::global_scope().implement_globals(*semantics, gologpp::ReadylogContext::instance());
	history->attach_semantics(*semantics);
}
