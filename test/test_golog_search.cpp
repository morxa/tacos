/***************************************************************************
 *  test_golog_search.cpp - Search over a Golog++ program
 *
 *  Created:   Tue 19 Oct 14:32:54 CEST 2021
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
#include "gocos/golog_program.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "parser/parser.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "semantics/readylog/execution.h"
#include "visualization/tree_to_graphviz.h"

#include <semantics/readylog/history.h>
#include <semantics/readylog/utilities.h>
#include <spdlog/spdlog.h>

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <variant>

namespace {

using namespace tacos::logic;
using tacos::search::GologProgram;

using AP = AtomicProposition<std::string>;

TEST_CASE("Search on a simple golog program", "[golog]")
{
	spdlog::set_level(spdlog::level::trace);
	GologProgram program(R"(
    action say() { }
    action yell() { }
    action hear() { }
    action env_terminate() {}
    procedure main() { hear(); choose { say(); yell(); } }
  )");
	const auto   f = finally(MTLFormula<std::string>{std::string{"start(say())"}})
	               || finally(MTLFormula<std::string>{std::string{"start(env_terminate())"}})
	               || MTLFormula<std::string>{std::string{"start(env_terminate())"}};
	auto ata = tacos::mtl_ata_translation::translate(f,
	                                                 std::set{
	                                                   AP{"start(say())"},
	                                                   AP{"end(say())"},
	                                                   AP{"start(yell())"},
	                                                   AP{"end(yell())"},
	                                                   AP{"start(hear())"},
	                                                   AP{"end(hear())"},
	                                                   AP{"start(env_terminate())"},
	                                                   AP{"end(env_terminate())"},
	                                                   AP{"start(ctl_terminate())"},
	                                                   AP{"end(ctl_terminate())"},
	                                                 });
	CAPTURE(ata);
	tacos::search::
	  TreeSearch<tacos::search::GologLocation, std::string, std::string, false, GologProgram>
	    search(&program,
	           &ata,
	           {"start(hear())",
	            "start(say())",
	            "start(yell())",
	            "start(ctl_terminate())",
	            "end(ctl_terminate())"},
	           {"end(hear())",
	            "end(say())",
	            "end(yell())",
	            "start(env_terminate())",
	            "end(env_terminate())"},
	           1,
	           true,
	           true);
	REQUIRE_NOTHROW(search.build_tree(false));
	tacos::visualization::search_tree_to_graphviz(*search.get_root())
	  .render_to_file("golog_tree.png");

	CHECK(search.get_root()->label == tacos::search::NodeLabel::TOP);
}

} // namespace
