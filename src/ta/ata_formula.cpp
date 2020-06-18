/***************************************************************************
 *  ata_formula.cpp - Alternating Timed Automata Formulas
 *
 *  Created: Thu 28 May 2020 14:41:01 CEST 14:41
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

#include <ta/ata_formula.h>
#include <ta/automata.h>

#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>

namespace automata {
namespace ata {

bool
TrueFormula::is_satisfied(const std::set<State> &, const ClockValuation &) const
{
	return true;
}

std::set<std::set<State>>
TrueFormula::get_minimal_models(const ClockValuation &) const
{
	return {{}};
}

bool
FalseFormula::is_satisfied(const std::set<State> &, const ClockValuation &) const
{
	return false;
}

std::set<std::set<State>>
FalseFormula::get_minimal_models(const ClockValuation &) const
{
	return {};
}

LocationFormula::LocationFormula(const Location &location) : location_(location)
{
}

bool
LocationFormula::is_satisfied(const std::set<State> &states, const ClockValuation &v) const
{
	return states.count(std::make_pair(location_, v));
}

std::set<std::set<State>>
LocationFormula::get_minimal_models(const ClockValuation &v) const
{
	return {{State(location_, v)}};
}

ClockConstraintFormula::ClockConstraintFormula(const ClockConstraint &constraint)
: constraint_(constraint)
{
}

bool
ClockConstraintFormula::is_satisfied(const std::set<State> &, const ClockValuation &v) const
{
	return automata::is_satisfied(constraint_, v);
}

std::set<std::set<State>>
ClockConstraintFormula::get_minimal_models(const ClockValuation &v) const
{
	if (automata::is_satisfied(constraint_, v)) {
		return {{}};
	} else {
		return {};
	}
}

ConjunctionFormula::ConjunctionFormula(std::unique_ptr<Formula> conjunct1,
                                       std::unique_ptr<Formula> conjunct2)
: conjunct1_(std::move(conjunct1)), conjunct2_(std::move(conjunct2))
{
}

bool
ConjunctionFormula::is_satisfied(const std::set<State> &states, const ClockValuation &v) const
{
	return conjunct1_->is_satisfied(states, v) && conjunct2_->is_satisfied(states, v);
}

std::set<std::set<State>>
ConjunctionFormula::get_minimal_models(const ClockValuation &v) const
{
	auto                      s1        = conjunct1_->get_minimal_models(v);
	auto                      s2        = conjunct2_->get_minimal_models(v);
	auto                      cartesian = ranges::views::cartesian_product(s1, s2);
	std::set<std::set<State>> res;
	ranges::for_each(cartesian, [&](const auto &prod) {
		std::set<State> u = std::get<0>(prod);
		u.insert(std::get<1>(prod).begin(), std::get<1>(prod).end());
		res.insert(u);
	});
	return res;
}

DisjunctionFormula::DisjunctionFormula(std::unique_ptr<Formula> disjunct1,
                                       std::unique_ptr<Formula> disjunct2)
: disjunct1_(std::move(disjunct1)), disjunct2_(std::move(disjunct2))
{
}

bool
DisjunctionFormula::is_satisfied(const std::set<State> &states, const ClockValuation &v) const
{
	return disjunct1_->is_satisfied(states, v) || disjunct2_->is_satisfied(states, v);
}

std::set<std::set<State>>
DisjunctionFormula::get_minimal_models(const ClockValuation &v) const
{
	auto disjunct1_models = disjunct1_->get_minimal_models(v);
	auto disjunct2_models = disjunct2_->get_minimal_models(v);
	disjunct1_models.insert(disjunct2_models.begin(), disjunct2_models.end());
	return disjunct1_models;
}

ResetClockFormula::ResetClockFormula(std::unique_ptr<Formula> sub_formula)
: sub_formula_(std::move(sub_formula))
{
}

bool
ResetClockFormula::is_satisfied(const std::set<State> &states, const ClockValuation &) const
{
	return sub_formula_->is_satisfied(states, 0);
}

std::set<std::set<State>>
ResetClockFormula::get_minimal_models(const ClockValuation &) const
{
	return sub_formula_->get_minimal_models(0);
}

} // namespace ata
} // namespace automata

std::ostream &
operator<<(std::ostream &os, const automata::ata::State &state)
{
	os << std::string("(") << state.first << std::string(",") << std::to_string(state.second)
	   << std::string(")");
	return os;
}
