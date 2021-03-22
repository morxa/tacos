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

#include "mtl/MTLFormula.h"
#include "mtl_ata_translation/translator.h"
#include "utilities/Interval.h"

#include <catch2/catch_all.hpp>
#include <stdexcept>

namespace {

using logic::MTLFormula;
using logic::TimeInterval;
using mtl_ata_translation::translate;
using utilities::arithmetic::BoundType;

using AP = logic::AtomicProposition<std::string>;

TEST_CASE("ATA satisfiability of simple MTL formulas", "[translator]")
{
	const MTLFormula a{AP{"a"}};
	const MTLFormula b{AP{"b"}};
	const MTLFormula c{AP{"c"}};
	const MTLFormula d{AP{"d"}};

	SECTION("A simple until formula")
	{
		const MTLFormula phi = a.until(b);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"a", 0}, {"a", 1}, {"b", 2.5}}));
		CHECK(ata.accepts_word({{"a", 0}, {"a", 1}, {"b", 1.5}}));
		CHECK(!ata.accepts_word({{"c", 0}, {"b", 1.5}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1.5}}));
		CHECK(!ata.accepts_word({{"b", 0}}));
		CHECK(ata.accepts_word({{"b", 0}, {"b", 0}}));
		CHECK(ata.accepts_word({{"b", 0}, {"b", 1}}));
	}

	SECTION("True literal in MTL formula")
	{
		const MTLFormula phi = MTLFormula<std::string>::TRUE().until(b);
		const auto       ata = translate(phi, {AP{"a"}, AP{"b"}});
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"a", 0}, {"a", 1}, {"b", 2}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 1}, {"a", 2}}));
	}

	SECTION("False literal in MTL formula")
	{
		const MTLFormula phi = MTLFormula<std::string>::FALSE().until(b);
		const auto       ata = translate(phi, {AP{"a"}, AP{"b"}});
		INFO("ATA:\n" << ata);
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 1}, {"b", 2}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 2}}));
	}

	SECTION("An until formula with time bounds")
	{
		const MTLFormula phi = a.until(b, TimeInterval(2, 3));
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}, {"b", 2}}));
		CHECK(ata.accepts_word({{"a", 0}, {"a", 1}, {"b", 3}}));
		CHECK(ata.accepts_word({{"a", 0}, {"a", 2.9}, {"b", 3}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 1}, {"b", 3.1}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 5}, {"b", 7}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 0.1}, {"b", 1.9}}));
	}

	SECTION("An until formula with strict lower time bound")
	{
		const MTLFormula phi = a.until(b, TimeInterval(2, BoundType::STRICT, 2, BoundType::INFTY));
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}, {"b", 2.1}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 0.5}, {"b", 2}}));
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}, {"a", 5}, {"a", 10}, {"b", 12}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 12}}));
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.1}, {"b", 12}}));
	}

	SECTION("An until formula with strict upper bound")
	{
		const MTLFormula phi = a.until(b, TimeInterval(2, BoundType::WEAK, 3, BoundType::STRICT));
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}, {"b", 2}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 1}, {"b", 3}}));
	}

	SECTION("An until with a negation")
	{
		const MTLFormula phi = (!a).until(b);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		// TODO(morxa) this is broken because there is no c in the automaton
		// CHECK(ata.accepts_word({{"c", 0}, {"c", 1}, {"b", 2.5}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 1}, {"b", 1.5}}));
	}

	SECTION("An until with a disjunctive subformula")
	{
		const MTLFormula phi = (a || b).until(c);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}, {"c", 1}}));
		CHECK(ata.accepts_word({{"b", 0}, {"b", 0.5}, {"c", 1}}));
		CHECK(ata.accepts_word({{"b", 0}, {"a", 0.5}, {"b", 0.8}, {"c", 1}}));
	}

	SECTION("An until with a conjunctive subformula")
	{
		const MTLFormula phi = (a && b).until(c);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 0.5}, {"c", 1}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 0.5}, {"c", 1}}));
		CHECK(ata.accepts_word({{"a", 0}, {"c", 0.5}, {"c", 1}}));
	}

	SECTION("An until with a conjunctive subformula with negations")
	{
		const MTLFormula phi = (!a && !b).until(c);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 0.5}, {"c", 1}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 0.5}, {"c", 1}}));
		CHECK(ata.accepts_word({{"c", 0}, {"c", 0.5}, {"c", 1}}));
		// TODO(morxa) this is broken because there is no c in the automaton
		// CHECK(ata.accepts_word({{"a", 0}, {"d", 0.5}, {"c", 1}}));
	}

	SECTION("An until with a negation of a non-atomic formula")
	{
		const MTLFormula phi = (!(a && b)).until(c);
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"a", 0}, {"a", 0.5}, {"c", 1}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 0.5}, {"c", 1}}));
		CHECK(ata.accepts_word({{"c", 0}, {"c", 0.5}, {"c", 1}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"a", 0.5}, {"a", 1}}));
	}

	SECTION("Nested until")
	{
		const MTLFormula phi = a.until(b.until(c));
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"c", 3}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"c", 1}, {"b", 1}}));
	}

	SECTION("Nested until with time bounds")
	{
		const MTLFormula phi = a.until(b.until(c, TimeInterval(1, 2)), TimeInterval(0, 1));
		const auto       ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"c", 3}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 1}, {"c", 1.5}}));
	}

	SECTION("Dual until")
	{
		const MTLFormula phi = a.dual_until(b);
		// ~ (~a U ~b)
		const auto ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"b", 0}}));
		CHECK(ata.accepts_word({{"b", 0}, {"b", 1}, {"b", 2}}));
		CHECK(ata.accepts_word({{"b", 0}, {"b", 1}, {"b", 2}, {"b", 3}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 2}, {"b", 3}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 2}, {"b", 3}, {"a", 4}}));
	}

	SECTION("Dual until with time bounds")
	{
		const MTLFormula phi = a.dual_until(b, TimeInterval(2, 3));
		// ~ (~a U ~b)
		const auto ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"b", 0}, {"b", 1}, {"b", 3.0}, {"b", 4}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 3.0}, {"b", 4}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 3.1}, {"b", 4}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 1.9}, {"b", 4}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 1.9}, {"a", 2.5}, {"b", 4}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 2.0}, {"b", 4}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 2}, {"b", 3}, {"a", 4}}));
	}

	SECTION("Dual until with strict time bounds")
	{
		const MTLFormula phi =
		  a.dual_until(b, TimeInterval(2, BoundType::STRICT, 3, BoundType::STRICT));
		// ~ (~a U ~b)
		const auto ata = translate(phi);
		INFO("ATA:\n" << ata);
		CHECK(ata.accepts_word({{"b", 0}, {"b", 1}, {"b", 3.0}, {"b", 4}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 2.9}, {"b", 4}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 3.0}, {"b", 4}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 3.1}, {"b", 4}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 1.9}, {"b", 4}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 1.9}, {"a", 2.5}, {"b", 4}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 2.0}, {"b", 4}}));
		CHECK(!ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 2.1}, {"b", 4}}));
		CHECK(ata.accepts_word({{"a", 0}, {"b", 1}, {"a", 2}, {"b", 3}, {"a", 4}}));
	}
}

TEST_CASE("MTL ATA Translation exceptions", "[translator][exceptions]")
{
	CHECK_THROWS_AS(translate(MTLFormula{AP{"phi_i"}}), std::invalid_argument);
}

} // namespace
