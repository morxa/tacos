/***************************************************************************
 *  ata_formula.hpp - Alternating Timed Automata Formulas
 *
 *  Created:   Fri 12 Feb 13:19:28 CET 2021
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

#pragma once

#include "ata_formula.h"

namespace automata::ata {

template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const Formula<LocationT> &formula)
{
	formula.print_to_ostream(os);
	return os;
}

template <typename LocationT>
bool
TrueFormula<LocationT>::is_satisfied(const std::set<State<LocationT>> &,
                                     const ClockValuation &) const
{
	return true;
}

template <typename LocationT>
std::set<std::set<State<LocationT>>>
TrueFormula<LocationT>::get_minimal_models(const ClockValuation &) const
{
	return {{}};
}

template <typename LocationT>
void
TrueFormula<LocationT>::print_to_ostream(std::ostream &os) const
{
	os << u8"⊤";
}

template <typename LocationT>
bool
FalseFormula<LocationT>::is_satisfied(const std::set<State<LocationT>> &,
                                      const ClockValuation &) const
{
	return false;
}

template <typename LocationT>
std::set<std::set<State<LocationT>>>
FalseFormula<LocationT>::get_minimal_models(const ClockValuation &) const
{
	return {};
}

template <typename LocationT>
void
FalseFormula<LocationT>::print_to_ostream(std::ostream &os) const
{
	os << u8"⊥";
}

template <typename LocationT>
bool
LocationFormula<LocationT>::is_satisfied(const std::set<State<LocationT>> &states,
                                         const ClockValuation &            v) const
{
	return states.count(std::make_pair(location_, v));
}

template <typename LocationT>
std::set<std::set<State<LocationT>>>
LocationFormula<LocationT>::get_minimal_models(const ClockValuation &v) const
{
	return {{std::make_pair(location_, v)}};
}

template <typename LocationT>
void
LocationFormula<LocationT>::print_to_ostream(std::ostream &os) const
{
	os << location_;
}

template <typename LocationT>
bool
ClockConstraintFormula<LocationT>::is_satisfied(const std::set<State<LocationT>> &,
                                                const ClockValuation &v) const
{
	return automata::is_satisfied(constraint_, v);
}

template <typename LocationT>
std::set<std::set<State<LocationT>>>
ClockConstraintFormula<LocationT>::get_minimal_models(const ClockValuation &v) const
{
	if (automata::is_satisfied(constraint_, v)) {
		return {{}};
	} else {
		return {};
	}
}

template <typename LocationT>
void
ClockConstraintFormula<LocationT>::print_to_ostream(std::ostream &os) const
{
	os << "x " << constraint_;
}

template <typename LocationT>
bool
ConjunctionFormula<LocationT>::is_satisfied(const std::set<State<LocationT>> &states,
                                            const ClockValuation &            v) const
{
	return conjunct1_->is_satisfied(states, v) && conjunct2_->is_satisfied(states, v);
}

template <typename LocationT>
std::set<std::set<State<LocationT>>>
ConjunctionFormula<LocationT>::get_minimal_models(const ClockValuation &v) const
{
	auto                                 s1        = conjunct1_->get_minimal_models(v);
	auto                                 s2        = conjunct2_->get_minimal_models(v);
	auto                                 cartesian = ranges::views::cartesian_product(s1, s2);
	std::set<std::set<State<LocationT>>> res;
	ranges::for_each(cartesian, [&](const auto &prod) {
		auto u = std::get<0>(prod);
		u.insert(std::get<1>(prod).begin(), std::get<1>(prod).end());
		res.insert(u);
	});
	return res;
}

template <typename LocationT>
void
ConjunctionFormula<LocationT>::print_to_ostream(std::ostream &os) const
{
	os << "(" << *conjunct1_ << u8" ∧ " << *conjunct2_ << ")";
}

template <typename LocationT>
bool
DisjunctionFormula<LocationT>::is_satisfied(const std::set<State<LocationT>> &states,
                                            const ClockValuation &            v) const
{
	return disjunct1_->is_satisfied(states, v) || disjunct2_->is_satisfied(states, v);
}

template <typename LocationT>
std::set<std::set<State<LocationT>>>
DisjunctionFormula<LocationT>::get_minimal_models(const ClockValuation &v) const
{
	auto disjunct1_models = disjunct1_->get_minimal_models(v);
	auto disjunct2_models = disjunct2_->get_minimal_models(v);
	disjunct1_models.insert(disjunct2_models.begin(), disjunct2_models.end());
	return disjunct1_models;
}

template <typename LocationT>
void
DisjunctionFormula<LocationT>::print_to_ostream(std::ostream &os) const
{
	os << "(" << *disjunct1_ << u8" ∨ " << *disjunct2_ << ")";
}

template <typename LocationT>
bool
ResetClockFormula<LocationT>::is_satisfied(const std::set<State<LocationT>> &states,
                                           const ClockValuation &) const
{
	return sub_formula_->is_satisfied(states, 0);
}

template <typename LocationT>
std::set<std::set<State<LocationT>>>
ResetClockFormula<LocationT>::get_minimal_models(const ClockValuation &) const
{
	return sub_formula_->get_minimal_models(0);
}

template <typename LocationT>
void
ResetClockFormula<LocationT>::print_to_ostream(std::ostream &os) const
{
	os << "x." << *sub_formula_;
}

} // namespace automata::ata

template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const automata::ata::State<LocationT> &state)
{
	os << "(" << state.first << ", " << state.second << std::string(")");
	return os;
}
