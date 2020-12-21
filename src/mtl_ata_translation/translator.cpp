/***************************************************************************
 *  translator.cpp - Translate an MTL formula into an ATA
 *
 *  Created: Thu 18 Jun 2020 11:21:13 CEST 11:21
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
#include "ta/ata_formula.h"
#include "ta/automata.h"

#include <mtl_ata_translation/translator.h>
#include <ta/ata.h>

#include <memory>
#include <stdexcept>

namespace mtl_ata_translation {

using namespace automata;
using logic::AtomicProposition;
using logic::LOP;
using logic::MTLFormula;
using logic::TimeInterval;
using logic::TimePoint;

// In this context:
// 1. Formulas are always ATA formulas over MTLFormulas.
using Formula                = ata::Formula<MTLFormula<ActionType>>;
using TrueFormula            = ata::TrueFormula<MTLFormula<ActionType>>;
using FalseFormula           = ata::FalseFormula<MTLFormula<ActionType>>;
using ConjunctionFormula     = ata::ConjunctionFormula<MTLFormula<ActionType>>;
using ResetClockFormula      = ata::ResetClockFormula<MTLFormula<ActionType>>;
using DisjunctionFormula     = ata::DisjunctionFormula<MTLFormula<ActionType>>;
using LocationFormula        = ata::LocationFormula<MTLFormula<ActionType>>;
using ClockConstraintFormula = ata::ClockConstraintFormula<MTLFormula<ActionType>>;
// 2. The resulting type is an ATA over MTLFormulas and AtomicPropositions.
using AlternatingTimedAutomaton =
  ata::AlternatingTimedAutomaton<MTLFormula<ActionType>, AtomicProposition<std::string>>;
using Transition = ata::Transition<MTLFormula<ActionType>, AtomicProposition<std::string>>;
using utilities::arithmetic::BoundType;

namespace {

std::set<MTLFormula<ActionType>>
get_closure(const MTLFormula<ActionType> &formula)
{
	auto untils      = formula.get_subformulas_of_type(LOP::LUNTIL);
	auto dual_untils = formula.get_subformulas_of_type(LOP::LDUNTIL);
	untils.insert(dual_untils.begin(), dual_untils.end());
	return untils;
}

/// Creates constraint defining the passed interval
std::unique_ptr<Formula>
create_contains(TimeInterval duration)
{
	std::unique_ptr<Formula> lowerBound = std::make_unique<TrueFormula>();
	std::unique_ptr<Formula> upperBound = std::make_unique<TrueFormula>();
	if (duration.lowerBoundType() != BoundType::INFTY) {
		if (duration.lowerBoundType() == BoundType::WEAK) {
			lowerBound = std::make_unique<ClockConstraintFormula>(
			  AtomicClockConstraintT<std::greater_equal<TimePoint>>(duration.lower()));
		} else {
			lowerBound = std::make_unique<ClockConstraintFormula>(
			  AtomicClockConstraintT<std::greater<TimePoint>>(duration.lower()));
		}
	}
	if (duration.upperBoundType() != BoundType::INFTY) {
		if (duration.upperBoundType() == BoundType::WEAK) {
			upperBound = std::make_unique<ClockConstraintFormula>(
			  AtomicClockConstraintT<std::less_equal<TimePoint>>(duration.upper()));
		} else {
			upperBound = std::make_unique<ClockConstraintFormula>(
			  AtomicClockConstraintT<std::less<TimePoint>>(duration.upper()));
		}
	}
	return std::make_unique<ConjunctionFormula>(std::move(lowerBound), std::move(upperBound));
}

/// Creates constraint excluding the passed interval
std::unique_ptr<Formula>
create_negated_contains(TimeInterval duration)
{
	std::unique_ptr<Formula> lowerBound = std::make_unique<FalseFormula>();
	std::unique_ptr<Formula> upperBound = std::make_unique<FalseFormula>();
	if (duration.lowerBoundType() != BoundType::INFTY) {
		if (duration.lowerBoundType() == BoundType::WEAK) {
			lowerBound = std::make_unique<ClockConstraintFormula>(
			  AtomicClockConstraintT<std::less<TimePoint>>(duration.lower()));
		} else {
			lowerBound = std::make_unique<ClockConstraintFormula>(
			  AtomicClockConstraintT<std::less_equal<TimePoint>>(duration.lower()));
		}
	}
	if (duration.upperBoundType() != BoundType::INFTY) {
		if (duration.upperBoundType() == BoundType::WEAK) {
			upperBound = std::make_unique<ClockConstraintFormula>(
			  AtomicClockConstraintT<std::greater<TimePoint>>(duration.upper()));
		} else {
			upperBound = std::make_unique<ClockConstraintFormula>(
			  AtomicClockConstraintT<std::greater_equal<TimePoint>>(duration.upper()));
		}
	}
	return std::make_unique<DisjunctionFormula>(std::move(lowerBound), std::move(upperBound));
}

std::unique_ptr<Formula>
init(const MTLFormula<ActionType> &formula, const AtomicProposition<ActionType> &ap)
{
	switch (formula.get_operator()) {
	case LOP::LUNTIL:
	case LOP::LDUNTIL:
		// init(psi, a) = x.psi if psi \in cl(phi)
		return std::make_unique<ResetClockFormula>(std::make_unique<LocationFormula>(formula));
	case LOP::LAND:
		// init(psi_1 AND psi_2, a) = init(psi_1, a) AND init(psi_2, a)
		return std::make_unique<ConjunctionFormula>(init(formula.get_operands().front(), ap),
		                                            init(formula.get_operands().back(), ap));
	case LOP::LOR:
		// init(psi_1 OR psi_2, a) = init(psi_1, a) OR init(psi_2, a)
		return std::make_unique<DisjunctionFormula>(init(formula.get_operands().front(), ap),
		                                            init(formula.get_operands().back(), ap));
	case LOP::AP:
		if (formula == ap) {
			// init(b, a) = TRUE if b == a
			return std::make_unique<TrueFormula>();
		} else {
			// init(b, a) = FALSE if b != a
			return std::make_unique<FalseFormula>();
		}
	case LOP::LNEG:
		// init(NOT b, a) = NOT init(b, a)
		// ATA formulas do not have negations, directly compute the result.
		// We know that this is an atomic proposition because the input formula is in positive normal
		// form.
		if (formula.get_operands().front() == ap) {
			// init(b, a) = TRUE if b == a
			return std::make_unique<FalseFormula>();
		} else {
			// init(b, a) = FALSE if b != a
			return std::make_unique<TrueFormula>();
		}
	}
	throw std::logic_error("Unexpected formula operator");
}

} // namespace

AlternatingTimedAutomaton
translate(const MTLFormula<ActionType> &input_formula)
{
	const auto formula = input_formula.to_positive_normal_form();
	// The ATA alphabet is the same as the formula alphabet.
	const auto alphabet = formula.get_alphabet();
	if (alphabet.count({"phi_i"}) > 0) {
		throw std::invalid_argument("The formula alphabet must not contain the symbol 'phi_i'");
	}
	// S = cl(phi) U {phi_i}
	auto locations = get_closure(formula);
	locations.insert({AtomicProposition<ActionType>{"phi_i"}});
	const auto           untils              = formula.get_subformulas_of_type(LOP::LUNTIL);
	const auto           dual_untils         = formula.get_subformulas_of_type(LOP::LDUNTIL);
	const auto           accepting_locations = dual_untils;
	std::set<Transition> transitions;
	for (const auto &symbol : alphabet) {
		// Initial transition delta(phi_i, symbol) -> phi
		transitions.insert(Transition(AtomicProposition<ActionType>{"phi_i"},
		                              symbol.ap_,
		                              std::make_unique<LocationFormula>(formula)));

		for (const auto &until : untils) {
			auto transition_formula = std::make_unique<DisjunctionFormula>(
			  // init(phi_2, symbol) AND x \in I
			  std::make_unique<ConjunctionFormula>(init(until.get_operands().back(), symbol),
			                                       create_contains(until.get_interval())),
			  // init(phi_1, symbol) AND (psi_1 \until_I psi_2)
			  //                         \->   == until     <-/
			  std::make_unique<ConjunctionFormula>(init(until.get_operands().front(), symbol),
			                                       std::make_unique<LocationFormula>(until)));
			transitions.insert(Transition(until, symbol, std::move(transition_formula)));
		}
		for (const auto &dual_until : dual_untils) {
			auto transition_formula = std::make_unique<DisjunctionFormula>(
			  std::make_unique<ConjunctionFormula>(init(dual_until.get_operands().back(), symbol),
			                                       create_negated_contains(dual_until.get_interval())),
			  std::make_unique<ConjunctionFormula>(init(dual_until.get_operands().front(), symbol),
			                                       std::make_unique<LocationFormula>(dual_until)));
			transitions.insert(Transition(dual_until, symbol, std::move(transition_formula)));
		}
	}
	return AlternatingTimedAutomaton(alphabet,
	                                 MTLFormula<ActionType>{{"phi_i"}},
	                                 accepting_locations,
	                                 std::move(transitions));
}
} // namespace mtl_ata_translation
