/***************************************************************************
 *  test_mtl_ata_translation.cpp
 *
 *  Created: Mon 29 Jun 2020 16:33:49 CEST 16:33
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "utilities/Interval.h"

#include <spdlog/spdlog.h>

#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

namespace {

using namespace tacos;

using logic::MTLFormula;
using logic::TimeInterval;
using mtl_ata_translation::translate;
using utilities::arithmetic::BoundType;

using AP = logic::AtomicProposition<std::string>;
// using Word = automata::ata::TimedATAWord<AP>;

const AP a{"a"};
const AP b{"b"};
const AP c{"c"};
const AP d{"d"};

TEST_CASE("ATA satisfiability of simple MTL formulas", "[translator]")
{
	spdlog::set_level(spdlog::level::trace);

	SECTION("A simple until formula")
	{
		const MTLFormula phi = MTLFormula{a}.until(b);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{a, 0}, {a, 1}, {b, 2.5}}));
		CHECK(ata.accepts_word({{a, 0}, {a, 1}, {b, 1.5}}));
		CHECK(!ata.accepts_word({{c, 0}, {b, 1.5}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1.5}}));
		CHECK(!ata.accepts_word({{b, 0}}));
		CHECK(ata.accepts_word({{b, 0}, {b, 0}}));
		CHECK(ata.accepts_word({{b, 0}, {b, 1}}));
	}

	SECTION("True literal in MTL formula")
	{
		const MTLFormula phi = MTLFormula<std::string>::TRUE().until(b);
		const auto       ata = translate(phi, {AP{a}, AP{b}});
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{a, 0}, {a, 1}, {b, 2}}));
		CHECK(!ata.accepts_word({{a, 0}, {a, 1}, {a, 2}}));
	}

	SECTION("False literal in MTL formula")
	{
		const MTLFormula phi = MTLFormula<std::string>::FALSE().until(b);
		const auto       ata = translate(phi, {AP{a}, AP{b}});
		INFO("ATA:\n" << ata);
		CHECK(!ata.accepts_word({{a, 0}, {a, 1}, {b, 2}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 2}}));
	}

	SECTION("An until formula with time bounds")
	{
		const MTLFormula phi = MTLFormula{a}.until(b, TimeInterval(2, 3));
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{a, 0}, {a, 0.5}, {b, 2}}));
		CHECK(ata.accepts_word({{a, 0}, {a, 1}, {b, 3}}));
		CHECK(ata.accepts_word({{a, 0}, {a, 2.9}, {b, 3}}));
		CHECK(!ata.accepts_word({{a, 0}, {a, 1}, {b, 3.1}}));
		CHECK(!ata.accepts_word({{a, 0}, {a, 5}, {b, 7}}));
		CHECK(!ata.accepts_word({{a, 0}, {a, 0.1}, {b, 1.9}}));
	}

	SECTION("An until formula with strict lower time bound")
	{
		const MTLFormula phi =
		  MTLFormula{a}.until(b, TimeInterval(2, BoundType::STRICT, 2, BoundType::INFTY));
		const auto ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{a, 0}, {a, 0.5}, {b, 2.1}}));
		CHECK(!ata.accepts_word({{a, 0}, {a, 0.5}, {b, 2}}));
		CHECK(ata.accepts_word({{a, 0}, {a, 0.5}, {a, 5}, {a, 10}, {b, 12}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 12}}));
		CHECK(ata.accepts_word({{a, 0}, {a, 0.1}, {b, 12}}));
	}

	SECTION("An until formula with strict upper bound")
	{
		const MTLFormula phi =
		  MTLFormula{a}.until(b, TimeInterval(2, BoundType::WEAK, 3, BoundType::STRICT));
		const auto ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{a, 0}, {a, 0.5}, {b, 2}}));
		CHECK(!ata.accepts_word({{a, 0}, {a, 1}, {b, 3}}));
	}

	SECTION("An until with a negation")
	{
		const MTLFormula phi = (!a).until(b);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		// TODO(morxa) this is broken because there is no c in the automaton
		// CHECK(ata.accepts_word({{c, 0}, {c, 1}, {b, 2.5}}));
		CHECK(!ata.accepts_word({{a, 0}, {a, 1}, {b, 1.5}}));
	}

	SECTION("An until with a disjunctive subformula")
	{
		const MTLFormula phi = (a || b).until(c);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{a, 0}, {a, 0.5}, {c, 1}}));
		CHECK(ata.accepts_word({{b, 0}, {b, 0.5}, {c, 1}}));
		CHECK(ata.accepts_word({{b, 0}, {a, 0.5}, {b, 0.8}, {c, 1}}));
	}

	SECTION("An until with a conjunctive subformula")
	{
		const MTLFormula phi = (a && b).until(c);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(!ata.accepts_word({{a, 0}, {a, 0.5}, {c, 1}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 0.5}, {c, 1}}));
		CHECK(ata.accepts_word({{a, 0}, {c, 0.5}, {c, 1}}));
	}

	SECTION("An until with a conjunctive subformula with negations")
	{
		const MTLFormula phi = (!a && !b).until(c);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(!ata.accepts_word({{a, 0}, {a, 0.5}, {c, 1}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 0.5}, {c, 1}}));
		CHECK(ata.accepts_word({{c, 0}, {c, 0.5}, {c, 1}}));
		// TODO(morxa) this is broken because there is no c in the automaton
		// CHECK(ata.accepts_word({{a, 0}, {d, 0.5}, {c, 1}}));
	}

	SECTION("An until with a negation of a non-atomic formula")
	{
		const MTLFormula phi = (!(a && b)).until(c);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{a, 0}, {a, 0.5}, {c, 1}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 0.5}, {c, 1}}));
		CHECK(ata.accepts_word({{c, 0}, {c, 0.5}, {c, 1}}));
		CHECK(!ata.accepts_word({{a, 0}, {a, 0.5}, {a, 1}}));
	}

	SECTION("Nested until")
	{
		const MTLFormula phi = MTLFormula{a}.until(MTLFormula{b}.until(c));
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {c, 3}}));
		CHECK(!ata.accepts_word({{a, 0}, {c, 1}, {b, 1}}));
	}

	SECTION("Nested until with time bounds")
	{
		const MTLFormula phi =
		  MTLFormula{a}.until(MTLFormula{b}.until(c, TimeInterval(1, 2)), TimeInterval(0, 1));
		const auto ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {c, 3}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 1}, {c, 1.5}}));
	}

	SECTION("Dual until")
	{
		const MTLFormula phi = MTLFormula{a}.dual_until(b);
		// ~ (~a U ~b)
		const auto ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{b, 0}}));
		CHECK(ata.accepts_word({{b, 0}, {b, 1}, {b, 2}}));
		CHECK(ata.accepts_word({{b, 0}, {b, 1}, {b, 2}, {b, 3}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 1}, {a, 2}, {b, 3}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 1}, {a, 2}, {b, 3}, {a, 4}}));
	}

	SECTION("Dual until with time bounds")
	{
		const MTLFormula phi = MTLFormula{a}.dual_until(b, TimeInterval(2, 3));
		// ~ (~a U ~b)
		const auto ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{b, 0}, {b, 1}, {b, 3.0}, {b, 4}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 1}, {a, 3.0}, {b, 4}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {a, 3.1}, {b, 4}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {a, 1.9}, {b, 4}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {a, 1.9}, {a, 2.5}, {b, 4}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 1}, {a, 2.0}, {b, 4}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 1}, {a, 2}, {b, 3}, {a, 4}}));
	}

	SECTION("Dual until with strict time bounds")
	{
		const MTLFormula phi =
		  MTLFormula{a}.dual_until(b, TimeInterval(2, BoundType::STRICT, 3, BoundType::STRICT));
		// ~ (~a U ~b)
		const auto ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{b, 0}, {b, 1}, {b, 3.0}, {b, 4}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 1}, {a, 2.9}, {b, 4}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {a, 3.0}, {b, 4}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {a, 3.1}, {b, 4}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {a, 1.9}, {b, 4}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {a, 1.9}, {a, 2.5}, {b, 4}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {a, 2.0}, {b, 4}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 1}, {a, 2.1}, {b, 4}}));
		CHECK(ata.accepts_word({{a, 0}, {b, 1}, {a, 2}, {b, 3}, {a, 4}}));
	}

	SECTION("Single negation operation")
	{
		const MTLFormula phi             = !a;
		const auto       ata             = translate(phi, {AP(a), AP(b), AP(c), AP(d)});
		const auto       ata_no_alphabet = translate(phi);

		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{b, 0}}));
		CHECK(ata.accepts_word({{b, 0}, {c, 1}}));
		CHECK(ata.accepts_word({{b, 0}, {a, 1}}));
		CHECK(!ata.accepts_word({{a, 0}}));
		CHECK(!ata.accepts_word({{a, 0}, {b, 1}}));

		CHECK(!ata_no_alphabet.accepts_word({{b, 0}}));
		CHECK(!ata_no_alphabet.accepts_word({{b, 0}, {c, 1}}));
		CHECK(!ata_no_alphabet.accepts_word({{b, 0}, {a, 1}}));
		CHECK(!ata_no_alphabet.accepts_word({{a, 0}}));
		CHECK(!ata_no_alphabet.accepts_word({{a, 0}, {b, 1}}));
	}

	SECTION("Single conjunction of two different APs")
	{
		// TODO Conjunction of AP does not really make sense, since we can only read single symbols on a
		// transition
		const MTLFormula phi = a && b;
		const auto       ata = translate(phi, {AP(a), AP(b), AP(c), AP(d)});

		INFO("ATA:\n" << ata);
		CHECK(!ata.accepts_word({{b, 0}}));
		CHECK(!ata.accepts_word({{a, 0}}));
	}

	SECTION("Single conjunction of two similar APs")
	{
		const MTLFormula phi = a && a;
		const auto       ata = translate(phi, {AP(a), AP(b), AP(c), AP(d)});

		INFO("ATA:\n" << ata);
		CHECK(!ata.accepts_word({{b, 0}}));
		CHECK(ata.accepts_word({{a, 0}}));
	}

	SECTION("Single disjunction of two APs")
	{
		const MTLFormula phi = a || b;
		const auto       ata = translate(phi, {AP(a), AP(b), AP(c), AP(d)});

		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{b, 0}}));
		CHECK(ata.accepts_word({{a, 0}}));
		CHECK(!ata.accepts_word({{c, 0}}));
		CHECK(!ata.accepts_word({{d, 0}}));
	}

	SECTION("Simple tautology")
	{
		const MTLFormula phi = MTLFormula{a} || !a;
		const auto       ata = translate(phi, {AP(a), AP(b), AP(c), AP(d)});

		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{b, 0}}));
		CHECK(ata.accepts_word({{a, 0}}));
		CHECK(ata.accepts_word({{c, 0}}));
		CHECK(ata.accepts_word({{d, 0}}));
	}

	SECTION("Simple unsatisfiable ata")
	{
		const MTLFormula phi = MTLFormula{a} && !a;
		const auto       ata = translate(phi, {AP(a), AP(b), AP(c), AP(d)});

		INFO("ATA:\n" << ata);
		CHECK(!ata.accepts_word({{b, 0}}));
		CHECK(!ata.accepts_word({{a, 0}}));
		CHECK(!ata.accepts_word({{c, 0}}));
		CHECK(!ata.accepts_word({{d, 0}}));
	}
}

TEST_CASE("MTL ATA Translation exceptions", "[translator][exceptions]")
{
	CHECK_THROWS_AS(translate(MTLFormula{AP{"l0"}}), std::invalid_argument);
	CHECK_THROWS_AS(translate(MTLFormula{AP{"sink"}}), std::invalid_argument);
}

TEST_CASE("MTL ATA sink location", "[translator][sink]")
{
	using Configuration = automata::ata::Configuration<MTLFormula<std::string>>;
	using State         = automata::ata::State<MTLFormula<std::string>>;
	// using Run           = automata::ata::Run<MTLFormula<std::string>, std::string>;
	// const std::string a_symbol{{a}};
	// const std::string b_symbol{{b}};
	// const std::string c_symbol{{c}};
	// const MTLFormula  a{AP{{a}}};
	// const MTLFormula  b{AP{{b}}};
	const MTLFormula sink{AP{"sink"}};

	SECTION("Sink location in the very first transition")
	{
		spdlog::set_level(spdlog::level::debug);
		const MTLFormula phi = MTLFormula{a} && !a;
		const auto       ata = translate(phi, {AP(a)});
		CAPTURE(ata);

		// After reading a, phi is no longer satisfiable.
		auto runs = ata.make_symbol_transition({{}}, a);
		// Thus, we should end up in the sink location.
		CAPTURE(runs);
		REQUIRE(runs.size() == 1);
		REQUIRE(runs[0].size() == 1);
		CHECK(runs[0][0].second == Configuration{{sink, 0}});

		// We should be able to loop in the sink location.
		runs = ata.make_symbol_transition(ata.make_time_transition(runs, 0), a);
		CAPTURE(runs);
		REQUIRE(runs.size() == 1);
		REQUIRE(runs[0].size() == 3);
		CHECK(runs[0][2].second == Configuration{{sink, 0}});
	}

	SECTION("Sink location for an until transition")
	{
		const MTLFormula phi = MTLFormula{a}.until(b);
		const auto       ata = translate(phi, {a, b, c});
		CAPTURE(ata);

		// After reading a -> 0 -> c, phi is no longer satisfiable.
		auto runs = ata.make_symbol_transition({{}}, a);
		CAPTURE(runs);
		runs = ata.make_time_transition(runs, 0);
		CAPTURE(runs);
		runs = ata.make_symbol_transition(runs, c);
		CAPTURE(runs);
		REQUIRE(runs.size() == 1);
		REQUIRE(runs[0].size() == 3);
		// Thus, we should end up in the sink location.
		CHECK(runs[0][2].second == Configuration{{sink, 0}});
	}

	SECTION("Sink location for a dual until transition")
	{
		const MTLFormula phi = MTLFormula{a}.dual_until(b);
		const auto       ata = translate(phi, {a, b, c});
		CAPTURE(ata);

		// After reading a -> 0 -> c, phi is no longer satisfiable.
		auto runs =
		  ata.make_symbol_transition(ata.make_time_transition(ata.make_symbol_transition({{}}, a), 0),
		                             c);
		CAPTURE(runs);
		REQUIRE(runs.size() == 1);
		REQUIRE(runs[0].size() == 3);
		// Thus, we should end up in the sink location.
		CHECK(runs[0][2].second.find(State{sink, 0}) != runs[0][2].second.end());
	}

	SECTION("Sink location for an until transition with a duration")
	{
		const MTLFormula phi = MTLFormula{a}.until(b, TimeInterval{0, 1});
		const auto       ata = translate(phi, {AP(a), AP(b), AP(c)});
		CAPTURE(ata);

		// After reading a -> 2 -> b, phi is no longer satisfiable.
		auto runs =
		  ata.make_symbol_transition(ata.make_time_transition(ata.make_symbol_transition({{}}, a), 2),
		                             b);
		CAPTURE(runs);
		REQUIRE(runs.size() == 1);
		REQUIRE(runs[0].size() == 3);
		// Thus, we should end up in the sink location.
		CHECK(runs[0][2].second == Configuration{{sink, 0}});
	}

	SECTION("Sink location for a dual until transition with a duration")
	{
		const MTLFormula phi = MTLFormula{a}.dual_until(b, TimeInterval{0, 1});
		const auto       ata = translate(phi, {AP(a), AP(b), AP(c)});
		CAPTURE(ata);

		// After reading a -> 0 -> b, phi is no longer satisfiable.
		auto runs =
		  ata.make_symbol_transition(ata.make_time_transition(ata.make_symbol_transition({{}}, a), 0),
		                             a);
		CAPTURE(runs);
		REQUIRE(runs.size() == 1);
		REQUIRE(runs[0].size() == 3);
		// Thus, we should end up in the sink location.
		CHECK(runs[0][2].second == Configuration{{sink, 0}});
	}
}

TEST_CASE("State-based MTL ATA Translation", "[translator]")
{
	using APSet = logic::AtomicProposition<std::set<std::string>>;
	const APSet      symbol_e{std::set<std::string>{}};
	const APSet      symbol_a{std::set<std::string>{"a"}};
	const APSet      symbol_ab{std::set<std::string>{"a", "b"}};
	const APSet      symbol_b{std::set<std::string>{"b"}};
	const MTLFormula sink{AP{"sink"}};
	const MTLFormula l0{AP{"l0"}};
	SECTION("Until with an Interval")
	{
		const MTLFormula phi      = MTLFormula{a}.until(b, TimeInterval(0, 1));
		const auto       alphabet = mtl_ata_translation::compute_alphabet<true, std::string>({a, b, c});
		const auto       ata      = translate<std::string, std::set<std::string>, true>(phi, alphabet);
		CAPTURE(ata);
		CHECK(ata.accepts_word({{symbol_a, 0}, {symbol_ab, 0.5}}));
		CHECK(ata.accepts_word({{symbol_a, 0}, {symbol_ab, 0.5}, {symbol_ab, 1.0}}));
		CHECK(ata.accepts_word({{symbol_a, 0}, {symbol_b, 0.5}, {symbol_ab, 1.0}}));
		CHECK(!ata.accepts_word({{symbol_a, 0}, {symbol_b, 1.5}, {symbol_ab, 2.0}}));
		CHECK(!ata.accepts_word({{symbol_a, 0}, {symbol_e, 0.5}, {symbol_ab, 0.8}}));
		CHECK(!ata.accepts_word({{symbol_a, 0}, {symbol_a, 0.5}, {symbol_a, 1.0}}));
	}
	SECTION("Until with negation")
	{
		const MTLFormula phi      = (!a).until(b);
		const auto       alphabet = mtl_ata_translation::compute_alphabet<true, std::string>({a, b, c});
		const auto       ata      = translate<std::string, std::set<std::string>, true>(phi, alphabet);
		CAPTURE(ata);
		CHECK(ata.accepts_word({{symbol_a, 0}, {symbol_ab, 0.5}}));
		CHECK(!ata.accepts_word({{symbol_a, 0}, {symbol_a, 0.5}, {symbol_b, 0.5}}));
		CHECK(ata.accepts_word({{symbol_a, 0}, {symbol_ab, 0.5}, {symbol_ab, 1.0}}));
		CHECK(ata.accepts_word({{symbol_a, 0}, {symbol_b, 0.5}, {symbol_a, 1.0}}));
		CHECK(ata.accepts_word({{symbol_a, 0}, {symbol_b, 0.5}, {symbol_ab, 1.0}}));
		CHECK(ata.accepts_word({{symbol_a, 0}, {symbol_e, 0.5}, {symbol_ab, 0.8}}));
		CHECK(!ata.accepts_word({{symbol_a, 0}, {symbol_e, 0.5}, {symbol_a, 0.6}, {symbol_ab, 0.8}}));
		CHECK(!ata.accepts_word({{symbol_a, 0}, {symbol_a, 0.5}, {symbol_a, 1.0}}));
	}
	SECTION("Sink")
	{
		const MTLFormula phi      = MTLFormula{a} && !a;
		const auto       alphabet = mtl_ata_translation::compute_alphabet<true, std::string>({a, b, c});
		const auto       ata      = translate<std::string, std::set<std::string>, true>(phi, alphabet);
		CAPTURE(ata);
		// After reading a, phi is no longer satisfiable.
		auto runs = ata.make_symbol_transition({{}}, APSet{{"a"}});
		// Thus, we should end up in the sink location.
		CAPTURE(runs);
		REQUIRE(runs.size() == 1);
		REQUIRE(runs[0].size() == 1);
		using Configuration = automata::ata::Configuration<MTLFormula<std::string>>;
		CHECK(runs[0][0].second == Configuration{{{sink}, 0}});
	}
	SECTION("Automatically derive alphabet from the input formula")
	{
		const MTLFormula phi = MTLFormula{a}.until(b);
		const auto       ata = translate<std::string, std::set<std::string>, true>(phi);
		CHECK(mtl_ata_translation::compute_alphabet<true, std::string>({a, b})
		      == std::set{APSet{{}}, APSet{{"a"}}, APSet{{"b"}}, APSet{{"a", "b"}}});
		CHECK(ata.get_alphabet() == mtl_ata_translation::compute_alphabet<true, std::string>({a, b}));
	}
	SECTION("Invalid sink symbol in the formula")
	{
		const MTLFormula phi = sink.until(b);
		CAPTURE(phi.get_alphabet());
		CHECK_THROWS(translate<std::string, std::set<std::string>, true>(phi));
	}
	SECTION("Invalid l0 symbol in the formula")
	{
		const MTLFormula phi = l0.until(b);
		CAPTURE(phi.get_alphabet());
		CHECK_THROWS(translate<std::string, std::set<std::string>, true>(phi));
	}
}

} // namespace
