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

#include "automata/ata_formula.h"
#include "automata/automata.h"

#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <memory>

namespace {

using namespace automata;
using namespace automata::ata;
using State = automata::ata::State<std::string>;

TEST_CASE("Simple ATA formulas", "[ta]")
{
	REQUIRE(TrueFormula<std::string>().is_satisfied({}, 0));
	REQUIRE(!FalseFormula<std::string>().is_satisfied({}, 0));
	REQUIRE(LocationFormula<std::string>("s1").is_satisfied({{"s0", 0}, {"s1", 0}}, 0));
	REQUIRE(!LocationFormula<std::string>("s1").is_satisfied({{"s0", 0}, {"s2", 0}}, 0));
	REQUIRE(!LocationFormula<std::string>("s1").is_satisfied({}, 0));
	REQUIRE(!LocationFormula<std::string>("s0").is_satisfied({{"s0", 0}}, 1));

	{
		ClockConstraint c = AtomicClockConstraintT<std::greater<Time>>(1);
		REQUIRE(ClockConstraintFormula<std::string>(c).is_satisfied({{"s0", 0}}, 2));
		REQUIRE(ClockConstraintFormula<std::string>(c).is_satisfied({{"s0", 2}}, 2));
		REQUIRE(!ClockConstraintFormula<std::string>(c).is_satisfied({{"s0", 2}}, 0));
		REQUIRE(!ClockConstraintFormula<std::string>(c).is_satisfied({{"s0", 0}}, 0));
	}
	{
		ClockConstraint c = AtomicClockConstraintT<std::less<Time>>(1);
		REQUIRE(!ClockConstraintFormula<std::string>(c).is_satisfied({{"s0", 0}}, 2));
		REQUIRE(!ClockConstraintFormula<std::string>(c).is_satisfied({{"s0", 2}}, 2));
		REQUIRE(ClockConstraintFormula<std::string>(c).is_satisfied({{"s0", 2}}, 0));
		REQUIRE(ClockConstraintFormula<std::string>(c).is_satisfied({{"s0", 0}}, 0));
	}
}

TEST_CASE("ATA conjunction formulas", "[ta]")
{
	REQUIRE(ConjunctionFormula<std::string>(std::make_unique<TrueFormula<std::string>>(),
	                                        std::make_unique<TrueFormula<std::string>>())
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(!ConjunctionFormula<std::string>(std::make_unique<TrueFormula<std::string>>(),
	                                         std::make_unique<FalseFormula<std::string>>())
	           .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(!ConjunctionFormula<std::string>(std::make_unique<FalseFormula<std::string>>(),
	                                         std::make_unique<TrueFormula<std::string>>())
	           .is_satisfied({{"s0", 0}}, 0));

	REQUIRE(ConjunctionFormula<std::string>(std::make_unique<LocationFormula<std::string>>("s0"),
	                                        std::make_unique<LocationFormula<std::string>>("s0"))
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(ConjunctionFormula<std::string>(std::make_unique<LocationFormula<std::string>>("s1"),
	                                        std::make_unique<LocationFormula<std::string>>("s2"))
	          .is_satisfied({{"s1", 0}, {"s2", 0}}, 0));
	REQUIRE(!ConjunctionFormula<std::string>(std::make_unique<LocationFormula<std::string>>("s1"),
	                                         std::make_unique<LocationFormula<std::string>>("s2"))
	           .is_satisfied({{"s1", 0}}, 0));
	REQUIRE(ConjunctionFormula<std::string>(std::make_unique<ConjunctionFormula<std::string>>(
	                                          std::make_unique<LocationFormula<std::string>>("s0"),
	                                          std::make_unique<LocationFormula<std::string>>("s1")),
	                                        std::make_unique<ConjunctionFormula<std::string>>(
	                                          std::make_unique<LocationFormula<std::string>>("s2"),
	                                          std::make_unique<LocationFormula<std::string>>("s3")))
	          .is_satisfied({{"s0", 0}, {"s1", 0}, {"s2", 0}, {"s3", 0}}, 0));
}

TEST_CASE("ATA disjunction formulas", "[ta]")
{
	REQUIRE(DisjunctionFormula<std::string>(std::make_unique<TrueFormula<std::string>>(),
	                                        std::make_unique<TrueFormula<std::string>>())
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(DisjunctionFormula<std::string>(std::make_unique<TrueFormula<std::string>>(),
	                                        std::make_unique<FalseFormula<std::string>>())
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(DisjunctionFormula<std::string>(std::make_unique<FalseFormula<std::string>>(),
	                                        std::make_unique<TrueFormula<std::string>>())
	          .is_satisfied({{"s0", 0}}, 0));

	REQUIRE(DisjunctionFormula<std::string>(std::make_unique<LocationFormula<std::string>>("s0"),
	                                        std::make_unique<LocationFormula<std::string>>("s0"))
	          .is_satisfied({{"s0", 0}}, 0));
	REQUIRE(DisjunctionFormula<std::string>(std::make_unique<LocationFormula<std::string>>("s1"),
	                                        std::make_unique<LocationFormula<std::string>>("s2"))
	          .is_satisfied({{"s1", 0}, {"s2", 0}}, 0));
	REQUIRE(DisjunctionFormula<std::string>(std::make_unique<LocationFormula<std::string>>("s1"),
	                                        std::make_unique<LocationFormula<std::string>>("s2"))
	          .is_satisfied({{"s1", 0}}, 0));
	REQUIRE(DisjunctionFormula<std::string>(std::make_unique<DisjunctionFormula<std::string>>(
	                                          std::make_unique<LocationFormula<std::string>>("s0"),
	                                          std::make_unique<LocationFormula<std::string>>("s1")),
	                                        std::make_unique<DisjunctionFormula<std::string>>(
	                                          std::make_unique<LocationFormula<std::string>>("s2"),
	                                          std::make_unique<LocationFormula<std::string>>("s3")))
	          .is_satisfied({{"s0", 0}, {"s1", 0}, {"s2", 0}, {"s3", 0}}, 0));
	REQUIRE(DisjunctionFormula<std::string>(std::make_unique<DisjunctionFormula<std::string>>(
	                                          std::make_unique<LocationFormula<std::string>>("s0"),
	                                          std::make_unique<LocationFormula<std::string>>("s1")),
	                                        std::make_unique<DisjunctionFormula<std::string>>(
	                                          std::make_unique<LocationFormula<std::string>>("s2"),
	                                          std::make_unique<LocationFormula<std::string>>("s3")))
	          .is_satisfied({{"s3", 0}}, 0));
}

TEST_CASE("ATA reset clock formulas", "[ta]")
{
	ResetClockFormula<std::string> l{std::make_unique<LocationFormula<std::string>>("s0")};
	REQUIRE(l.is_satisfied({{"s0", 0}}, 1));
	ResetClockFormula<std::string> f{std::make_unique<ClockConstraintFormula<std::string>>(
	  AtomicClockConstraintT<std::less<Time>>(1))};
	REQUIRE(f.is_satisfied({{"s1", 0}}, 2));
}

TEST_CASE("Minimal models of ATA atomic formulas", "[ta]")
{
	REQUIRE(TrueFormula<std::string>().get_minimal_models(2) == std::set<std::set<State>>{{}});
	REQUIRE(FalseFormula<std::string>().get_minimal_models(2) == std::set<std::set<State>>{});
	{
		LocationFormula<std::string> f{"s0"};
		REQUIRE(f.get_minimal_models(0) == std::set<std::set<State>>{{State{"s0", 0}}});
		REQUIRE(f.get_minimal_models(1) == std::set<std::set<State>>{{State{"s0", 1}}});
	}
	{
		ResetClockFormula<std::string> f(std::make_unique<LocationFormula<std::string>>("s0"));
		REQUIRE(f.get_minimal_models(1) == std::set<std::set<State>>{{State{"s0", 0}}});
	}
}
TEST_CASE("Minimal models of ATA conjunction formulas", "[ta]")
{
	{
		ConjunctionFormula<std::string> f(std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<LocationFormula<std::string>>("s1"));
		REQUIRE(f.get_minimal_models(0) == std::set<std::set<State>>{{State{"s0", 0}, State{"s1", 0}}});
		REQUIRE(f.get_minimal_models(1) == std::set<std::set<State>>{{State{"s0", 1}, State{"s1", 1}}});
	}
	{
		ConjunctionFormula<std::string> f(std::make_unique<TrueFormula<std::string>>(),
		                                  std::make_unique<FalseFormula<std::string>>());
		REQUIRE(f.get_minimal_models(0) == std::set<std::set<State>>{});
	}
	{
		ConjunctionFormula<std::string> f(std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<TrueFormula<std::string>>());
		REQUIRE(f.get_minimal_models(0) == std::set<std::set<State>>{{State{"s0", 0}}});
		REQUIRE(f.get_minimal_models(1) == std::set<std::set<State>>{{State{"s0", 1}}});
	}
	{
		ConjunctionFormula<std::string> f(std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<FalseFormula<std::string>>());
		REQUIRE(f.get_minimal_models(0) == std::set<std::set<State>>{});
		REQUIRE(f.get_minimal_models(1) == std::set<std::set<State>>{});
	}
	{
		ConjunctionFormula<std::string> f(std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<ResetClockFormula<std::string>>(
		                                    std::make_unique<LocationFormula<std::string>>("s1")));
		REQUIRE(f.get_minimal_models(1) == std::set<std::set<State>>{{State{"s0", 1}, State{"s1", 0}}});
	}
}
TEST_CASE("Minimal models of ATA disjunction formulas", "[ta]")
{
	{
		DisjunctionFormula<std::string> f(std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<LocationFormula<std::string>>("s1"));
		REQUIRE(f.get_minimal_models(0)
		        == std::set<std::set<State>>{{State{"s0", 0}}, {State{"s1", 0}}});
		REQUIRE(f.get_minimal_models(1)
		        == std::set<std::set<State>>{{State{"s0", 1}}, {State{"s1", 1}}});
	}
	{
		DisjunctionFormula<std::string> f(std::make_unique<TrueFormula<std::string>>(),
		                                  std::make_unique<FalseFormula<std::string>>());
		REQUIRE(f.get_minimal_models(0) == std::set<std::set<State>>{{}});
	}
	{
		DisjunctionFormula<std::string> f(std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<TrueFormula<std::string>>());
		REQUIRE(f.get_minimal_models(0) == std::set<std::set<State>>{{}});
		REQUIRE(f.get_minimal_models(1) == std::set<std::set<State>>{{}});
	}
	{
		DisjunctionFormula<std::string> f(std::make_unique<TrueFormula<std::string>>(),
		                                  std::make_unique<LocationFormula<std::string>>("s0"));
		REQUIRE(f.get_minimal_models(0) == std::set<std::set<State>>{{}});
		REQUIRE(f.get_minimal_models(1) == std::set<std::set<State>>{{}});
	}
	{
		DisjunctionFormula<std::string> f(std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<FalseFormula<std::string>>());
		REQUIRE(f.get_minimal_models(0) == std::set<std::set<State>>{{State{"s0", 0}}});
		REQUIRE(f.get_minimal_models(1) == std::set<std::set<State>>{{State{"s0", 1}}});
	}
	{
		DisjunctionFormula<std::string> f(std::make_unique<LocationFormula<std::string>>("s0"),
		                                  std::make_unique<ResetClockFormula<std::string>>(
		                                    std::make_unique<LocationFormula<std::string>>("s1")));
		REQUIRE(f.get_minimal_models(1)
		        == std::set<std::set<State>>{{State{"s0", 1}}, {State{"s1", 0}}});
	}
}

TEST_CASE("Compare ATA formulas", "[ta]")
{
	using T      = TrueFormula<std::string>;
	using F      = FalseFormula<std::string>;
	using C      = ConjunctionFormula<std::string>;
	using D      = DisjunctionFormula<std::string>;
	using L      = LocationFormula<std::string>;
	using ClockC = ClockConstraintFormula<std::string>;
	using R      = ResetClockFormula<std::string>;
	CHECK(T{} == T{});
	CHECK(F{} == F{});
	CHECK(T{} != F{});
	CHECK(T{} != C{std::make_unique<T>(), std::make_unique<T>()});
	CHECK(C{std::make_unique<T>(), std::make_unique<T>()}
	      == C{std::make_unique<T>(), std::make_unique<T>()});
	CHECK(C{std::make_unique<F>(), std::make_unique<T>()}
	      != C{std::make_unique<T>(), std::make_unique<T>()});
	CHECK(C{std::make_unique<F>(), std::make_unique<T>()}
	      != C{std::make_unique<T>(), std::make_unique<F>()});
	CHECK(D{std::make_unique<T>(), std::make_unique<T>()}
	      == D{std::make_unique<T>(), std::make_unique<T>()});
	CHECK(D{std::make_unique<F>(), std::make_unique<T>()}
	      != D{std::make_unique<T>(), std::make_unique<T>()});
	CHECK(D{std::make_unique<F>(), std::make_unique<T>()}
	      != D{std::make_unique<T>(), std::make_unique<F>()});
	CHECK(L{"a"} < L{"b"});
	CHECK(L{"a"} == L{"a"});
	CHECK(ClockC{AtomicClockConstraintT<std::greater<Time>>(1)}
	      < ClockC{AtomicClockConstraintT<std::greater<Time>>(2)});
	CHECK(ClockC{AtomicClockConstraintT<std::greater<Time>>(1)}
	      == ClockC{AtomicClockConstraintT<std::greater<Time>>(1)});
	CHECK(ClockC{AtomicClockConstraintT<std::less<Time>>(1)}
	      != ClockC{AtomicClockConstraintT<std::greater<Time>>(1)});
	CHECK(R{std::make_unique<T>()} == R{std::make_unique<T>()});
	CHECK(R{std::make_unique<T>()} != R{std::make_unique<F>()});
}

TEST_CASE("Create simplified formulas", "[ta]")
{
	using T = TrueFormula<std::string>;
	using F = FalseFormula<std::string>;
	using L = LocationFormula<std::string>;
	L l{"l"};

	CHECK(*create_conjunction<std::string>(std::make_unique<T>(), std::make_unique<T>()) == T{});
	CHECK(*create_conjunction<std::string>(std::make_unique<F>(), std::make_unique<F>()) == F{});
	CHECK(*create_conjunction<std::string>(std::make_unique<T>(), std::make_unique<F>()) == F{});
	CHECK(*create_conjunction<std::string>(std::make_unique<F>(), std::make_unique<T>()) == F{});

	CHECK(*create_conjunction<std::string>(std::make_unique<T>(), std::make_unique<L>("l")) == l);
	CHECK(*create_conjunction<std::string>(std::make_unique<F>(), std::make_unique<L>("l")) == F{});
	CHECK(*create_conjunction<std::string>(std::make_unique<L>("l"), std::make_unique<T>()) == l);
	CHECK(*create_conjunction<std::string>(std::make_unique<L>("l"), std::make_unique<F>()) == F{});

	CHECK(*create_disjunction<std::string>(std::make_unique<T>(), std::make_unique<T>()) == T{});
	CHECK(*create_disjunction<std::string>(std::make_unique<F>(), std::make_unique<F>()) == F{});
	CHECK(*create_disjunction<std::string>(std::make_unique<T>(), std::make_unique<F>()) == T{});
	CHECK(*create_disjunction<std::string>(std::make_unique<F>(), std::make_unique<T>()) == T{});

	CHECK(*create_disjunction<std::string>(std::make_unique<T>(), std::make_unique<L>("l")) == T{});
	CHECK(*create_disjunction<std::string>(std::make_unique<F>(), std::make_unique<L>("l")) == l);
	CHECK(*create_disjunction<std::string>(std::make_unique<L>("l"), std::make_unique<T>()) == T{});
	CHECK(*create_disjunction<std::string>(std::make_unique<L>("l"), std::make_unique<F>()) == l);
}

} // namespace
