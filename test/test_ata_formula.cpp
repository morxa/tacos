/***************************************************************************
 *  test_ata_formula.cpp - Test ATA formulas
 *
 *  Created: Thu 28 May 2020 16:33:29 CEST 16:33
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

#include "libta/automata.h"

#include <libta/ata_formula.h>

#include <catch2/catch.hpp>
#include <functional>
#include <memory>

using namespace automata;
using namespace automata::ata;

TEST_CASE("Simple ATA formulas", "[libta]")
{
	REQUIRE(TrueFormula().is_satisfied({}, 0));
	REQUIRE(!FalseFormula().is_satisfied({}, 0));
	REQUIRE(LocationFormula("s1").is_satisfied({{"s0", 0}, {"s1", 0}}, 0));
	REQUIRE(!LocationFormula("s1").is_satisfied({{"s0", 0}, {"s2", 0}}, 0));
	REQUIRE(!LocationFormula("s1").is_satisfied({}, 0));
	REQUIRE(!LocationFormula("s0").is_satisfied({{"s0", 0}}, 1));

	{
		ClockConstraint c = AtomicClockConstraintT<std::greater<Time>>(1);
		REQUIRE(ClockConstraintFormula(c).is_satisfied({{"s0", 0}}, 2));
		REQUIRE(ClockConstraintFormula(c).is_satisfied({{"s0", 2}}, 2));
		REQUIRE(!ClockConstraintFormula(c).is_satisfied({{"s0", 2}}, 0));
		REQUIRE(!ClockConstraintFormula(c).is_satisfied({{"s0", 0}}, 0));
	}
	{
		ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(1);
		REQUIRE(!ClockConstraintFormula(c).is_satisfied({{"s0", 0}}, 2));
		REQUIRE(!ClockConstraintFormula(c).is_satisfied({{"s0", 2}}, 2));
		REQUIRE(ClockConstraintFormula(c).is_satisfied({{"s0", 2}}, 0));
		REQUIRE(ClockConstraintFormula(c).is_satisfied({{"s0", 0}}, 0));
	}
}

TEST_CASE("ATA conjunction formulas", "[libta]")
{
	REQUIRE(ConjunctionFormula({}).is_satisfied({}, 0));
	std::vector<std::unique_ptr<Formula>> subs;
	SECTION("satisfying a conjunction with a single conjunct")
	{
		subs.emplace_back(std::make_unique<LocationFormula>("s0"));
		REQUIRE(ConjunctionFormula(std::move(subs)).is_satisfied({{"s0", 0}}, 0));
	}
	SECTION("satisfying two true conjuncts")
	{
		subs.emplace_back(std::make_unique<TrueFormula>());
		subs.emplace_back(std::make_unique<TrueFormula>());
		REQUIRE(ConjunctionFormula(std::move(subs)).is_satisfied({{"s0", 0}}, 0));
	}
	SECTION("not satisfying a true and a false conjunct")
	{
		subs.emplace_back(std::make_unique<TrueFormula>());
		subs.emplace_back(std::make_unique<FalseFormula>());
		REQUIRE(!ConjunctionFormula(std::move(subs)).is_satisfied({{"s0", 0}}, 0));
	}
	SECTION("satisfying two location conjuncts")
	{
		subs.emplace_back(std::make_unique<LocationFormula>("s1"));
		subs.emplace_back(std::make_unique<LocationFormula>("s2"));
		REQUIRE(DisjunctionFormula(std::move(subs)).is_satisfied({{"s1", 0}, {"s2", 1}}, 0));
	}
	SECTION("not satisfying a conjunction of one true and one false location formula")
	{
		subs.emplace_back(std::make_unique<LocationFormula>("s1"));
		subs.emplace_back(std::make_unique<LocationFormula>("s2"));
		REQUIRE(!ConjunctionFormula(std::move(subs)).is_satisfied({{"s1", 0}}, 0));
	}
}

TEST_CASE("ATA disjunction formulas", "[libta]")
{
	REQUIRE(!DisjunctionFormula({}).is_satisfied({}, 0));
	std::vector<std::unique_ptr<Formula>> subs;
	SECTION("satisfying a disjunction with a single disjunct")
	{
		subs.emplace_back(std::make_unique<LocationFormula>("s0"));
		REQUIRE(DisjunctionFormula(std::move(subs)).is_satisfied({{"s0", 0}}, 0));
	}
	SECTION("satisfying two true disjuncts")
	{
		subs.emplace_back(std::make_unique<TrueFormula>());
		subs.emplace_back(std::make_unique<TrueFormula>());
		REQUIRE(DisjunctionFormula(std::move(subs)).is_satisfied({{"s0", 0}}, 0));
	}
	SECTION("satisfying a true and a false disjuncts")
	{
		subs.emplace_back(std::make_unique<TrueFormula>());
		subs.emplace_back(std::make_unique<FalseFormula>());
		REQUIRE(DisjunctionFormula(std::move(subs)).is_satisfied({{"s0", 0}}, 0));
	}
	SECTION("satisfying two true location disjuncts")
	{
		subs.emplace_back(std::make_unique<LocationFormula>("s1"));
		subs.emplace_back(std::make_unique<LocationFormula>("s2"));
		REQUIRE(DisjunctionFormula(std::move(subs)).is_satisfied({{"s1", 0}, {"s2", 1}}, 0));
	}
	SECTION("satisfying a disjunction of one true and one false location formula")
	{
		subs.emplace_back(std::make_unique<LocationFormula>("s1"));
		subs.emplace_back(std::make_unique<LocationFormula>("s2"));
		REQUIRE(DisjunctionFormula(std::move(subs)).is_satisfied({{"s1", 0}}, 0));
	}
}

TEST_CASE("ATA reset clock formulas", "[libta]")
{
	ResetClockFormula l{std::make_unique<LocationFormula>("s0")};
	REQUIRE(l.is_satisfied({{"s0", 0}}, 1));
	ResetClockFormula f{
	  std::make_unique<ClockConstraintFormula>(AtomicClockConstraintT<std::less<Time>>(1))};
	REQUIRE(f.is_satisfied({{"s1", 0}}, 2));
}
