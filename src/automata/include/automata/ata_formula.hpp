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

#include <experimental/set>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <typeindex>

namespace tacos::automata::ata {

template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const automata::ata::Formula<LocationT> &formula)
{
	formula.print_to_ostream(os);
	return os;
}

template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const automata::ata::State<LocationT> &state)
{
	os << "(" << state.location << ", " << state.clock_valuation << std::string(")");
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
	return states.count(State<LocationT>{location_, v});
}

template <typename LocationT>
std::set<std::set<State<LocationT>>>
LocationFormula<LocationT>::get_minimal_models(const ClockValuation &v) const
{
	return {{State<LocationT>{location_, v}}};
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
	// Remove each model from disjunct1_models that is a superset of a model in disjunct2_models
	std::experimental::erase_if(disjunct1_models, [&disjunct2_models](const auto &model1) {
		return std::find_if(disjunct2_models.begin(),
		                    disjunct2_models.end(),
		                    [&model1](const auto &model2) {
			                    // True if model1 is a superset of model2.
			                    return std::includes(model1.begin(),
			                                         model1.end(),
			                                         model2.begin(),
			                                         model2.end());
		                    })
		       != disjunct2_models.end();
	});
	// Add each model from disjunct2_models which is not a superset of a model from disjunct1_models.
	for (const auto &model2 : disjunct2_models) {
		if (std::find_if(disjunct1_models.begin(),
		                 disjunct1_models.end(),
		                 [&model2](const auto &model1) {
			                 // True if model2 is a superset of model1.
			                 return std::includes(model2.begin(),
			                                      model2.end(),
			                                      model1.begin(),
			                                      model1.end());
		                 })
		    == disjunct1_models.end()) {
			disjunct1_models.insert(model2);
		}
	}
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

template <typename LocationT>
bool
operator<(const Formula<LocationT> &first, const Formula<LocationT> &second)
{
	if (typeid(first) != typeid(second)) {
		return std::type_index(typeid(first)) < std::type_index(typeid(second));
	}
	if (typeid(first) == typeid(TrueFormula<LocationT>)) {
		return false;
	}
	if (typeid(first) == typeid(FalseFormula<LocationT>)) {
		return false;
	}
	if (typeid(first) == typeid(ConjunctionFormula<LocationT>)) {
		const ConjunctionFormula<LocationT> &f1 =
		  static_cast<const ConjunctionFormula<LocationT> &>(first);
		const ConjunctionFormula<LocationT> &f2 =
		  static_cast<const ConjunctionFormula<LocationT> &>(second);
		return std::tie(*f1.conjunct1_, *f1.conjunct2_) < std::tie(*f2.conjunct1_, *f2.conjunct2_);
	}
	if (typeid(first) == typeid(DisjunctionFormula<LocationT>)) {
		const DisjunctionFormula<LocationT> &f1 =
		  static_cast<const DisjunctionFormula<LocationT> &>(first);
		const DisjunctionFormula<LocationT> &f2 =
		  static_cast<const DisjunctionFormula<LocationT> &>(second);
		return std::tie(*f1.disjunct1_, *f1.disjunct2_) < std::tie(*f2.disjunct1_, *f2.disjunct2_);
	}
	if (typeid(first) == typeid(LocationFormula<LocationT>)) {
		const LocationFormula<LocationT> &f1 = static_cast<const LocationFormula<LocationT> &>(first);
		const LocationFormula<LocationT> &f2 = static_cast<const LocationFormula<LocationT> &>(second);
		return f1.location_ < f2.location_;
	}
	if (typeid(first) == typeid(ClockConstraintFormula<LocationT>)) {
		const ClockConstraintFormula<LocationT> &f1 =
		  static_cast<const ClockConstraintFormula<LocationT> &>(first);
		const ClockConstraintFormula<LocationT> &f2 =
		  static_cast<const ClockConstraintFormula<LocationT> &>(second);
		return f1.constraint_ < f2.constraint_;
	}
	if (typeid(first) == typeid(ResetClockFormula<LocationT>)) {
		const ResetClockFormula<LocationT> &f1 =
		  static_cast<const ResetClockFormula<LocationT> &>(first);
		const ResetClockFormula<LocationT> &f2 =
		  static_cast<const ResetClockFormula<LocationT> &>(second);
		return *f1.sub_formula_ < *f2.sub_formula_;
	}
	throw std::logic_error("Unexpected formulas in comparison");
}

template <typename LocationT>
bool
operator==(const Formula<LocationT> &first, const Formula<LocationT> &second)
{
	return !(first < second) && !(second < first);
}

template <typename LocationT>
bool
operator!=(const Formula<LocationT> &first, const Formula<LocationT> &second)
{
	return (first < second) || (second < first);
}

template <typename LocationT>
std::unique_ptr<Formula<LocationT>>
create_conjunction(std::unique_ptr<Formula<LocationT>> conjunct1,
                   std::unique_ptr<Formula<LocationT>> conjunct2)
{
	if (*conjunct1 == FalseFormula<LocationT>{} || *conjunct2 == FalseFormula<LocationT>{}) {
		return std::make_unique<FalseFormula<LocationT>>();
	}
	if (*conjunct1 == TrueFormula<LocationT>{}) {
		return conjunct2;
	}
	if (*conjunct2 == TrueFormula<LocationT>{}) {
		return conjunct1;
	}
	return std::make_unique<ConjunctionFormula<LocationT>>(std::move(conjunct1),
	                                                       std::move(conjunct2));
}

template <typename LocationT>
std::unique_ptr<Formula<LocationT>>
create_disjunction(std::unique_ptr<Formula<LocationT>> disjunct1,
                   std::unique_ptr<Formula<LocationT>> disjunct2)
{
	if (*disjunct1 == TrueFormula<LocationT>{} || *disjunct2 == TrueFormula<LocationT>{}) {
		return std::make_unique<TrueFormula<LocationT>>();
	}
	if (*disjunct1 == FalseFormula<LocationT>{}) {
		return disjunct2;
	}
	if (*disjunct2 == FalseFormula<LocationT>{}) {
		return disjunct1;
	}
	return std::make_unique<DisjunctionFormula<LocationT>>(std::move(disjunct1),
	                                                       std::move(disjunct2));
}

} // namespace tacos::automata::ata
