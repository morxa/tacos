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

#include <libta/ata_formula.h>
#include <libta/automata.h>

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
	REQUIRE(ConjunctionFormula(std::make_unique<TrueFormula>(), std::make_unique<TrueFormula>())
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(!ConjunctionFormula(std::make_unique<TrueFormula>(), std::make_unique<FalseFormula>())
	           .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(!ConjunctionFormula(std::make_unique<FalseFormula>(), std::make_unique<TrueFormula>())
	           .is_satisfied({{"s0", 0}}, 0));

	REQUIRE(ConjunctionFormula(std::make_unique<LocationFormula>("s0"),
	                           std::make_unique<LocationFormula>("s0"))
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(ConjunctionFormula(std::make_unique<LocationFormula>("s1"),
	                           std::make_unique<LocationFormula>("s2"))
	          .is_satisfied({{"s1", 0}, {"s2", 0}}, 0));
	REQUIRE(!ConjunctionFormula(std::make_unique<LocationFormula>("s1"),
	                            std::make_unique<LocationFormula>("s2"))
	           .is_satisfied({{"s1", 0}}, 0));
	REQUIRE(ConjunctionFormula(
	          std::make_unique<ConjunctionFormula>(std::make_unique<LocationFormula>("s0"),
	                                               std::make_unique<LocationFormula>("s1")),
	          std::make_unique<ConjunctionFormula>(std::make_unique<LocationFormula>("s2"),
	                                               std::make_unique<LocationFormula>("s3")))
	          .is_satisfied({{"s0", 0}, {"s1", 0}, {"s2", 0}, {"s3", 0}}, 0));
}

TEST_CASE("ATA disjunction formulas", "[libta]")
{
	REQUIRE(DisjunctionFormula(std::make_unique<TrueFormula>(), std::make_unique<TrueFormula>())
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(DisjunctionFormula(std::make_unique<TrueFormula>(), std::make_unique<FalseFormula>())
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(DisjunctionFormula(std::make_unique<FalseFormula>(), std::make_unique<TrueFormula>())
	          .is_satisfied({{"s0", 0}}, 0));

	REQUIRE(DisjunctionFormula(std::make_unique<LocationFormula>("s0"),
	                           std::make_unique<LocationFormula>("s0"))
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(DisjunctionFormula(std::make_unique<LocationFormula>("s1"),
	                           std::make_unique<LocationFormula>("s2"))
	          .is_satisfied({{"s1", 0}, {"s2", 0}}, 0));
	REQUIRE(DisjunctionFormula(std::make_unique<LocationFormula>("s1"),
	                           std::make_unique<LocationFormula>("s2"))
	          .is_satisfied({{"s1", 0}}, 0));
	REQUIRE(DisjunctionFormula(
	          std::make_unique<DisjunctionFormula>(std::make_unique<LocationFormula>("s0"),
	                                               std::make_unique<LocationFormula>("s1")),
	          std::make_unique<DisjunctionFormula>(std::make_unique<LocationFormula>("s2"),
	                                               std::make_unique<LocationFormula>("s3")))
	          .is_satisfied({{"s0", 0}, {"s1", 0}, {"s2", 0}, {"s3", 0}}, 0));
	REQUIRE(DisjunctionFormula(
	          std::make_unique<DisjunctionFormula>(std::make_unique<LocationFormula>("s0"),
	                                               std::make_unique<LocationFormula>("s1")),
	          std::make_unique<DisjunctionFormula>(std::make_unique<LocationFormula>("s2"),
	                                               std::make_unique<LocationFormula>("s3")))
	          .is_satisfied({{"s3", 0}}, 0));
}

TEST_CASE("ATA reset clock formulas", "[libta]")
{
	ResetClockFormula l{std::make_unique<LocationFormula>("s0")};
	REQUIRE(l.is_satisfied({{"s0", 0}}, 1));
	ResetClockFormula f{
	  std::make_unique<ClockConstraintFormula>(AtomicClockConstraintT<std::less<Time>>(1))};
	REQUIRE(f.is_satisfied({{"s1", 0}}, 2));
}
