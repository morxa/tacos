/***************************************************************************
 *  mtl_proto.cpp - Protobuf importer for MTLFormulas
 *
 *  Created:   Sat 20 Mar 18:46:10 CET 2021
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

#include "mtl/mtl_proto.h"

#include "mtl/MTLFormula.h"
#include "mtl/mtl.pb.h"
#include "utilities/Interval.h"

#include <stdexcept>

namespace logic {

namespace {

std::pair<utilities::arithmetic::BoundType, double>
parse_interval_endpoint(const proto::MTLFormula::Interval::Endpoint &endpoint)
{
	switch (endpoint.bound_type()) {
	case proto::MTLFormula_Interval_BoundType_WEAK:
		return std::make_pair(utilities::arithmetic::BoundType::WEAK, endpoint.value());
	case proto::MTLFormula_Interval_BoundType_STRICT:
		return std::make_pair(utilities::arithmetic::BoundType::STRICT, endpoint.value());
	default:
		throw std::invalid_argument("Invalid interval bound type: " + endpoint.ShortDebugString());
	}
}

TimeInterval
parse_interval(const proto::MTLFormula::Interval &interval_proto)
{
	TimeInterval interval;
	if (interval_proto.has_lower()) {
		const auto &[bound, value] = parse_interval_endpoint(interval_proto.lower());
		interval.set_lower(value, bound);
	}
	if (interval_proto.has_upper()) {
		const auto &[bound, value] = parse_interval_endpoint(interval_proto.upper());
		interval.set_upper(value, bound);
	}
	return interval;
}

} // namespace

MTLFormula<std::string>
parse_proto(const proto::MTLFormula &mtl_formula)
{
	if (mtl_formula.has_constant()) {
		switch (mtl_formula.constant().value()) {
		case proto::MTLFormula_ConstantValue_FALSE: return MTLFormula<std::string>::FALSE();
		case proto::MTLFormula_ConstantValue_TRUE: return MTLFormula<std::string>::TRUE();
		default:
			throw std::invalid_argument(
			  "Unknown constant value "
			  + proto::MTLFormula::ConstantValue_Name(mtl_formula.constant().value()));
		}
		return MTLFormula<std::string>::TRUE();
	}
	if (mtl_formula.has_atomic()) {
		return MTLFormula{AtomicProposition{mtl_formula.atomic().symbol()}};
	}
	if (mtl_formula.has_conjunction()) {
		std::vector<MTLFormula<std::string>> sub_formulas;
		std::transform(std::begin(mtl_formula.conjunction().conjuncts()),
		               std::end(mtl_formula.conjunction().conjuncts()),
		               std::back_inserter(sub_formulas),
		               [](const auto &sub_formula) { return parse_proto(sub_formula); });
		return MTLFormula<std::string>::create_conjunction(sub_formulas);
	}
	if (mtl_formula.has_disjunction()) {
		std::vector<MTLFormula<std::string>> sub_formulas;
		std::transform(std::begin(mtl_formula.disjunction().disjuncts()),
		               std::end(mtl_formula.disjunction().disjuncts()),
		               std::back_inserter(sub_formulas),
		               [](const auto &sub_formula) { return parse_proto(sub_formula); });
		return MTLFormula<std::string>::create_disjunction(sub_formulas);
	}
	if (mtl_formula.has_negation()) {
		if (!mtl_formula.negation().has_formula()) {
			throw std::invalid_argument("Negation formula without sub-formula: "
			                            + mtl_formula.ShortDebugString());
		}
		return !parse_proto(mtl_formula.negation().formula());
	}
	if (mtl_formula.has_until()) {
		if (!mtl_formula.until().has_front()) {
			throw std::invalid_argument("Until without front sub-formula: "
			                            + mtl_formula.ShortDebugString());
		}
		if (!mtl_formula.until().has_back()) {
			throw std::invalid_argument("Until without back sub-formula: "
			                            + mtl_formula.ShortDebugString());
		}
		auto         front = parse_proto(mtl_formula.until().front());
		auto         back  = parse_proto(mtl_formula.until().back());
		TimeInterval interval;
		if (mtl_formula.until().has_interval()) {
			interval = parse_interval(mtl_formula.until().interval());
		}
		return front.until(back, interval);
	}
	if (mtl_formula.has_dual_until()) {
		if (!mtl_formula.dual_until().has_front()) {
			throw std::invalid_argument("Until without front sub-formula: "
			                            + mtl_formula.ShortDebugString());
		}
		if (!mtl_formula.dual_until().has_back()) {
			throw std::invalid_argument("Until without back sub-formula: "
			                            + mtl_formula.ShortDebugString());
		}
		auto         front = parse_proto(mtl_formula.dual_until().front());
		auto         back  = parse_proto(mtl_formula.dual_until().back());
		TimeInterval interval;
		if (mtl_formula.dual_until().has_interval()) {
			interval = parse_interval(mtl_formula.dual_until().interval());
		}
		return front.dual_until(back, interval);
	}
	throw std::invalid_argument("Unknown formula type in proto " + mtl_formula.ShortDebugString());
}

} // namespace logic
