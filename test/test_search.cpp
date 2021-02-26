/***************************************************************************
 *  test_search.cpp - Test the main search algorithm
 *
 *  Created:   Mon  1 Feb 17:23:48 CET 2021
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

#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "synchronous_product/search.h"
#include "synchronous_product/search_tree.h"
#include "synchronous_product/synchronous_product.h"

#include <catch2/catch.hpp>

namespace {

using TreeSearch      = synchronous_product::TreeSearch<std::string, std::string>;
using TATransition    = automata::ta::Transition<std::string, std::string>;
using TA              = automata::ta::TimedAutomaton<std::string, std::string>;
using TAConfiguration = automata::ta::Configuration<std::string>;
using TreeSearch      = synchronous_product::TreeSearch<std::string, std::string>;
using CanonicalABWord = synchronous_product::CanonicalABWord<std::string, std::string>;
using TARegionState   = synchronous_product::TARegionState<std::string>;
using ATARegionState  = synchronous_product::ATARegionState<std::string>;
using AP              = logic::AtomicProposition<std::string>;
using synchronous_product::NodeLabel;
using synchronous_product::NodeState;

TEST_CASE("Search in an ABConfiguration tree", "[search]")
{
	using automata::AtomicClockConstraintT;
	using AP = logic::AtomicProposition<std::string>;
	using utilities::arithmetic::BoundType;
	TA ta{{"a", "b", "c"}, "l0", {"l0", "l1", "l2"}};
	ta.add_clock("x");
	ta.add_transition(TATransition(
	  "l0", "a", "l0", {{"x", AtomicClockConstraintT<std::greater<automata::Time>>(1)}}, {"x"}));
	ta.add_transition(
	  TATransition("l0", "b", "l1", {{"x", AtomicClockConstraintT<std::less<automata::Time>>(1)}}));
	ta.add_transition(TATransition("l0", "c", "l2"));
	ta.add_transition(TATransition("l2", "b", "l1"));
	logic::MTLFormula<std::string> a{AP("a")};
	logic::MTLFormula<std::string> b{AP("b")};

	logic::MTLFormula f   = a.until(b, logic::TimeInterval(2, BoundType::WEAK, 2, BoundType::INFTY));
	auto              ata = mtl_ata_translation::translate(f);
	TreeSearch        search(&ta, &ata, {"a"}, {"b", "c"}, 2);

	SECTION("The search tree is initialized correctly")
	{
		CHECK(search.get_root()->words
		      == std::set{CanonicalABWord(
		        {{TARegionState{"l0", "x", 0}, ATARegionState{logic::MTLFormula{AP{"phi_i"}}, 0}}})});
		CHECK(search.get_root()->state == NodeState::UNKNOWN);
		CHECK(search.get_root()->parent == nullptr);
		CHECK(search.get_root()->incoming_actions.empty());
		CHECK(search.get_root()->children.empty());
	}

	SECTION("The first step computes the right children")
	{
		REQUIRE(search.step());
		const auto &children = search.get_root()->children;
		INFO("Children of the root node:\n" << children);
		REQUIRE(children.size() == 3);
		CHECK(children[0]->words
		      == std::set{
		        CanonicalABWord({{TARegionState{"l0", "x", 0}}, {ATARegionState{a.until(b), 3}}}),
		        CanonicalABWord({{TARegionState{"l0", "x", 0}, ATARegionState{a.until(b), 4}}}),
		        CanonicalABWord({{TARegionState{"l0", "x", 0}}, {ATARegionState{a.until(b), 5}}})});
		CHECK(children[0]->incoming_actions == std::set<std::string>{"a"});
		CHECK(
		  children[1]->words
		  == std::set{CanonicalABWord({{TARegionState{"l1", "x", 0}, ATARegionState{a.until(b), 0}}})});
		CHECK(children[1]->incoming_actions == std::set<std::string>{"b"});
		CHECK(
		  children[2]->words
		  == std::set{CanonicalABWord({{TARegionState{"l1", "x", 1}, ATARegionState{a.until(b), 1}}})});
		CHECK(children[2]->incoming_actions == std::set<std::string>{"b"});
	}

	SECTION("The next steps compute the right children")
	{
		REQUIRE(search.step());
		INFO("Tree:\n" << *search.get_root());
		REQUIRE(search.step());
		INFO("Tree:\n" << *search.get_root());
		const auto &root_children = search.get_root()->children;
		REQUIRE(root_children.size() == std::size_t(3));

		{
			// Process first child of the root.
			// starts with [{(l0, x, 0), ((a U b), 3)}]
			const auto &children = root_children[0]->children;
			REQUIRE(children.size() == std::size_t(3));
			CHECK(children[0]->words
			      == std::set{
			        CanonicalABWord({{TARegionState{"l0", "x", 0}}, {ATARegionState{a.until(b), 5}}})});
			CHECK(children[0]->incoming_actions == std::set<std::string>{"a"});
			CHECK(children[1]->words == std::set{CanonicalABWord({{TARegionState{"l1", "x", 0}}})});
			CHECK(children[1]->incoming_actions == std::set<std::string>{"b"});
			CHECK(children[2]->words == std::set{CanonicalABWord({{TARegionState{"l1", "x", 1}}})});
			CHECK(children[2]->incoming_actions == std::set<std::string>{"b"});
			CHECK(root_children[0]->state == NodeState::UNKNOWN);
		}

		// Process second child of the root.
		REQUIRE(search.step());
		INFO("Tree:\n" << *search.get_root());
		CHECK(root_children[1]->children.empty()); // should be ({(l1, x, 0), ((a U b), 0)})
		// the node has no time-symbol successors (only time successors)
		CHECK(root_children[1]->state == NodeState::DEAD);

		// Process third child of the root.
		REQUIRE(search.step());
		INFO("Tree:\n" << *search.get_root());
		REQUIRE(root_children[2]->children.empty()); // should be ({(l1, x, 1), ((a U b), 1)})
		// the node has no time-symbol successors (only time successors)
		REQUIRE(root_children[2]->state == NodeState::DEAD);
	}

	SECTION("Compute the final tree")
	{
		// We do exactly 7 steps.
		for (size_t i = 0; i < 7; i++) {
			REQUIRE(search.step());
		}
		REQUIRE(!search.step());
		search.label();

		INFO("Tree:\n" << *search.get_root());
		REQUIRE(search.get_root()->children.size() == 3);
		REQUIRE(search.get_root()->children[0]->children.size() == 3);
		REQUIRE(search.get_root()->children[1]->children.size() == 0);
		REQUIRE(search.get_root()->children[2]->children.size() == 0);
		REQUIRE(search.get_root()->children[0]->children[0]->children.size() == 0);
		REQUIRE(search.get_root()->children[0]->children[1]->children.size() == 0);
		REQUIRE(search.get_root()->children[0]->children[2]->children.size() == 0);

		CHECK(search.get_root()->state == NodeState::UNKNOWN);
		CHECK(search.get_root()->children[0]->state == NodeState::UNKNOWN);
		CHECK(search.get_root()->children[1]->state == NodeState::DEAD);
		CHECK(search.get_root()->children[2]->state == NodeState::DEAD);
		CHECK(search.get_root()->children[0]->children[0]->state == NodeState::GOOD);
		CHECK(search.get_root()->children[0]->children[1]->state == NodeState::BAD);
		CHECK(search.get_root()->children[0]->children[2]->state == NodeState::BAD);

		CHECK(search.get_root()->label == NodeLabel::TOP);
		CHECK(search.get_root()->children[0]->label == NodeLabel::TOP);
		CHECK(search.get_root()->children[1]->label == NodeLabel::TOP);
		CHECK(search.get_root()->children[2]->label == NodeLabel::TOP);
		CHECK(search.get_root()->children[0]->children[0]->label == NodeLabel::TOP);
		CHECK(search.get_root()->children[0]->children[1]->label == NodeLabel::BOTTOM);
		CHECK(search.get_root()->children[0]->children[2]->label == NodeLabel::BOTTOM);
	}
}

} // namespace
