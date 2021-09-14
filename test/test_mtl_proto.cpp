/***************************************************************************
 *  test_mtl_proto.cpp - Tests for the MTL proto importer
 *
 *  Created:   Sat 20 Mar 23:53:40 CET 2021
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

#include "mtl/mtl.pb.h"
#include "utilities/Interval.h"

#include <google/protobuf/text_format.h>
#include <mtl/MTLFormula.h>
#include <mtl/mtl_proto.h>

#include <catch2/catch_test_macros.hpp>

namespace {

using namespace tacos;

using google::protobuf::TextFormat;
using AtomicProposition = logic::AtomicProposition<std::string>;
using MTLFormula        = logic::MTLFormula<std::string>;
using logic::parse_proto;
using logic::TimeInterval;
using utilities::arithmetic::BoundType;

TEST_CASE("Import MTL formulas from a proto", "[libmtl][proto]")
{
	logic::proto::MTLFormula proto_formula;
	MTLFormula               a{AtomicProposition{"a"}};
	MTLFormula               b{AtomicProposition{"b"}};
	MTLFormula               c{AtomicProposition{"c"}};

	SECTION("Constant True")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(constant { value: TRUE })pb", &proto_formula));
		CHECK(parse_proto(proto_formula) == MTLFormula::TRUE());
	}

	SECTION("Constant False")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(constant: { value: FALSE })pb", &proto_formula));
		CHECK(parse_proto(proto_formula) == MTLFormula::FALSE());
	}

	SECTION("Atomic formula")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(atomic { symbol: "a" })pb", &proto_formula));
		CHECK(parse_proto(proto_formula) == a);
	}

	SECTION("Conjunction")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(conjunction {
                                               conjuncts { atomic { symbol: "a" } }
                                               conjuncts { atomic { symbol: "b" } }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula) == (a && b));
	}

	SECTION("Conjunction with three sub-formulas")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(conjunction {
                                               conjuncts { atomic { symbol: "a" } }
                                               conjuncts { atomic { symbol: "b" } }
                                               conjuncts { atomic { symbol: "c" } }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula) == MTLFormula::create_conjunction({a, b, c}));
	}

	SECTION("Disjunction")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(disjunction {
                                               disjuncts { atomic { symbol: "a" } }
                                               disjuncts { atomic { symbol: "b" } }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula) == (a || b));
	}

	SECTION("Disjunction with three sub-formulas")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(disjunction {
                                               disjuncts { atomic { symbol: "a" } }
                                               disjuncts { atomic { symbol: "b" } }
                                               disjuncts { atomic { symbol: "c" } }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula) == MTLFormula::create_disjunction({a, b, c}));
	}

	SECTION("Negation")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(negation { formula { atomic { symbol: "a" } } })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula) == !a);
	}

	SECTION("Until without bounds")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(until {
                                               front { atomic { symbol: "a" } }
                                               back { atomic { symbol: "b" } }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula) == a.until(b));
	}

	SECTION("Dual until without bounds")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(dual_until {
                                               front { atomic { symbol: "a" } }
                                               back { atomic { symbol: "b" } }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula) == a.dual_until(b));
	}

	SECTION("Until with weak upper bound")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(until {
                                               front { atomic { symbol: "a" } }
                                               back { atomic { symbol: "b" } }
                                               interval { upper { value: 2 bound_type: WEAK } }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula)
		      == a.until(b, TimeInterval{0, BoundType::INFTY, 2, BoundType::WEAK}));
	}

	SECTION("Dual until with weak upper bound")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(dual_until {
                                               front { atomic { symbol: "a" } }
                                               back { atomic { symbol: "b" } }
                                               interval { upper { value: 2 bound_type: WEAK } }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula)
		      == a.dual_until(b, TimeInterval{0, BoundType::INFTY, 2, BoundType::WEAK}));
	}

	SECTION("Until with strict lower bound")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(until {
                                               front { atomic { symbol: "a" } }
                                               back { atomic { symbol: "b" } }
                                               interval { lower { value: 2 bound_type: STRICT } }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula)
		      == a.until(b, TimeInterval{2, BoundType::STRICT, 0, BoundType::INFTY}));
	}
	SECTION("Dual until with both lower and upper bound")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(dual_until {
                                               front { atomic { symbol: "a" } }
                                               back { atomic { symbol: "b" } }
                                               interval {
                                                 lower { value: 1 bound_type: STRICT }
                                                 upper { value: 2 bound_type: WEAK }
                                               }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula)
		      == a.dual_until(b, TimeInterval{1, BoundType::STRICT, 2, BoundType::WEAK}));
	}
	SECTION("Finally without bounds")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(finally { formula { atomic { symbol: "a" } } })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula) == finally(a));
	}
	SECTION("Finally with bounds")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(finally {
                                               formula { atomic { symbol: "a" } }
                                               interval {
                                                 lower { value: 1 bound_type: STRICT }
                                                 upper { value: 2 bound_type: WEAK }
                                               }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula)
		      == finally(a, TimeInterval{1, BoundType::STRICT, 2, BoundType::WEAK}));
	}
	SECTION("Globally without bounds")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(globally { formula { atomic { symbol: "a" } } })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula) == globally(a));
	}
	SECTION("Globally with bounds")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(globally {
                                               formula { atomic { symbol: "a" } }
                                               interval {
                                                 lower { value: 1 bound_type: STRICT }
                                                 upper { value: 2 bound_type: WEAK }
                                               }
                                             })pb",
		                                    &proto_formula));
		CHECK(parse_proto(proto_formula)
		      == globally(a, TimeInterval{1, BoundType::STRICT, 2, BoundType::WEAK}));
	}
}

TEST_CASE("Exceptions when importing invalid MTL protos", "[libmtl][proto]")
{
	logic::proto::MTLFormula proto_formula;

	SECTION("Until formula with missing operand")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(until { front { atomic { symbol: "a" } } })pb",
		                                    &proto_formula));
		CHECK_THROWS(parse_proto(proto_formula));
		REQUIRE(TextFormat::ParseFromString(R"pb(until { back { atomic { symbol: "a" } } })pb",
		                                    &proto_formula));
		CHECK_THROWS(parse_proto(proto_formula));
	}

	SECTION("Dual Until formula with missing operand")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(dual_until { front { atomic { symbol: "a" } } })pb",
		                                    &proto_formula));
		CHECK_THROWS(parse_proto(proto_formula));
		REQUIRE(TextFormat::ParseFromString(R"pb(dual_until { back { atomic { symbol: "a" } } })pb",
		                                    &proto_formula));
		CHECK_THROWS(parse_proto(proto_formula));
	}

	SECTION("Negation with missing subformula")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(negation {})pb", &proto_formula));
		CHECK_THROWS(parse_proto(proto_formula));
	}

	SECTION("Finally with missing subformula")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(finally {})pb", &proto_formula));
		CHECK_THROWS(parse_proto(proto_formula));
	}

	SECTION("Globally with missing subformula")
	{
		REQUIRE(TextFormat::ParseFromString(R"pb(globally {})pb", &proto_formula));
		CHECK_THROWS(parse_proto(proto_formula));
	}
}

} // namespace
