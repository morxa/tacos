/***************************************************************************
 *  test_railroad.cpp - A railroad example
 *
 *  Created:   Mon  1 Mar 17:18:27 CET 2021
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

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_product.h"
#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "synchronous_product/search.h"

#include <catch2/catch_test_macros.hpp>

namespace {

using Location   = automata::ta::Location<std::string>;
using TA         = automata::ta::TimedAutomaton<std::string, std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;
using automata::AtomicClockConstraintT;
using automata::Time;
using F  = logic::MTLFormula<std::string>;
using AP = logic::AtomicProposition<std::string>;
using synchronous_product::NodeLabel;
using TreeSearch = synchronous_product::TreeSearch<std::vector<std::string>, std::string>;

TEST_CASE("A single railroad crossing", "[railroad]")
{
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%t %v");
	std::set<std::string> crossing_actions{"start_close",
	                                       "finish_close",
	                                       "start_open",
	                                       "finish_open"};
	TA crossing{{Location{"OPEN"}, Location{"CLOSING"}, Location{"CLOSED"}, Location{"OPENING"}},
	            crossing_actions,
	            Location{"OPEN"},
	            {Location{"OPEN"}},
	            {"x"},
	            {Transition(Location{"OPEN"}, "start_close", Location{"CLOSING"}, {}, {"x"}),
	             Transition(Location{"CLOSING"},
	                        "finish_close",
	                        Location{"CLOSED"},
	                        {{"x", AtomicClockConstraintT<std::equal_to<Time>>(4)}},
	                        {"x"}),
	             Transition(Location{"CLOSED"},
	                        "start_open",
	                        Location{"OPENING"},
	                        {{"x", AtomicClockConstraintT<std::greater_equal<Time>>(1)}},
	                        {"x"}),
	             Transition(Location{"OPENING"},
	                        "finish_open",
	                        Location{"OPEN"},
	                        {{"x", AtomicClockConstraintT<std::equal_to<Time>>(3)}},
	                        {"x"})}};
	std::set<std::string> train_actions{"get_near", "enter", "leave"};
	TA           train{{Location{"FAR"}, Location{"NEAR"}, Location{"IN"}, Location{"BEHIND"}},
           train_actions,
           Location{"FAR"},
           {Location{"BEHIND"}},
           {"t"},
           {Transition(Location{"FAR"},
                       "get_near",
                       Location{"NEAR"},
                       {{"t", AtomicClockConstraintT<std::greater<Time>>(10)}},
                       {"t"}),
            Transition(Location{"NEAR"},
                       "enter",
                       Location{"IN"},
                       {{"t", AtomicClockConstraintT<std::greater<Time>>(2)}},
                       {"t"}),
            Transition(Location{"IN"},
                       "leave",
                       Location{"BEHIND"},
                       {{"t", AtomicClockConstraintT<std::equal_to<Time>>(1)}},
                       {"t"})}

  };
	auto         product = automata::ta::get_product<std::string, std::string>({crossing, train});
	std::set<AP> actions;
	std::set_union(begin(crossing_actions),
	               end(crossing_actions),
	               begin(train_actions),
	               end(train_actions),
	               inserter(actions, end(actions)));
	auto ata = mtl_ata_translation::translate(F{AP{"true"}}.until(!AP{"enter"} || AP{"finish_close"}),
	                                          actions);
	INFO("TA: " << product);
	INFO("ATA: " << ata);
	TreeSearch search{&product, &ata, crossing_actions, train_actions, 10, true, true};
	search.build_tree(true);
	std::stringstream str;
	print_to_ostream(str, *search.get_root(), true);
	INFO("Tree:\n" << str.rdbuf());
	search.label();
	CHECK(search.get_root()->label == NodeLabel::TOP);
}

} // namespace
