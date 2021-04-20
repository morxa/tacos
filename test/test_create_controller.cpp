/***************************************************************************
 *  test_create_controller.cpp - Test the created controller
 *
 *  Created:   Tue 06 Apr 17:09:55 CEST 2021
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

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_product.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "railroad.h"
#include "synchronous_product/create_controller.h"
#include "synchronous_product/search.h"
#include "synchronous_product/search_tree.h"

#ifdef HAVE_VISUALIZATION
#	include "visualization/ta_to_graphviz.h"
#endif

#include <catch2/catch_test_macros.hpp>

namespace {

using F          = logic::MTLFormula<std::string>;
using AP         = logic::AtomicProposition<std::string>;
using TreeSearch = synchronous_product::TreeSearch<std::vector<std::string>, std::string>;
using controller_synthesis::create_controller;
using synchronous_product::NodeLabel;

TEST_CASE("Create a controller for railroad1", "[railroad][controller]")
{
	const auto &[product, controller_actions, environment_actions] = create_crossing_problem({2});
	std::set<AP> actions;
	std::set_union(begin(controller_actions),
	               end(controller_actions),
	               begin(environment_actions),
	               end(environment_actions),
	               inserter(actions, end(actions)));
	const auto finish_close_1 = F{AP{"finish_close_1"}};
	const auto start_open_1   = F{AP{"start_open_1"}};
	const auto enter_1        = F{AP{"enter_1"}};
	auto       ata =
	  mtl_ata_translation::translate(((!finish_close_1).until(enter_1) && finally(enter_1)), actions);
	TreeSearch search{&product, &ata, controller_actions, environment_actions, 2, true, true};
	search.build_tree(true);
	CHECK(search.get_root()->label == NodeLabel::TOP);
	auto controller = create_controller(search.get_root());
#ifdef HAVE_VISUALIZATION
	visualization::ta_to_graphviz(controller).render_to_file("railroad1_controller.svg");
#endif
	// TODO Check more properties of the controller
	CHECK(controller.get_alphabet() == std::set<std::string>{"start_close_1", "finish_close_1"});
}
} // namespace
