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

namespace mtl_ata_translation {

std::set<logic::MTLFormula<ActionType>>
get_closure(const logic::MTLFormula<ActionType> &formula)
{
	auto untils      = formula.get_subformulas_of_type(logic::LOP::LUNTIL);
	auto dual_untils = formula.get_subformulas_of_type(logic::LOP::LDUNTIL);
	untils.insert(dual_untils.begin(), dual_untils.end());
	return untils;
}

std::unique_ptr<automata::ata::Formula<logic::MTLFormula<ActionType>>>
  create_contains(logic::TimeInterval)
{
	// TODO implement
	return std::make_unique<automata::ata::TrueFormula<logic::MTLFormula<ActionType>>>();
}

std::unique_ptr<automata::ata::Formula<logic::MTLFormula<ActionType>>>
  create_negated_contains(logic::TimeInterval)
{
	// TODO implement
	return std::make_unique<automata::ata::FalseFormula<logic::MTLFormula<ActionType>>>();
}

std::unique_ptr<automata::ata::Formula<logic::MTLFormula<ActionType>>>
init(const logic::MTLFormula<ActionType> &formula, const logic::AtomicProposition<ActionType> &ap)
{
	switch (formula.get_operator()) {
	case logic::LOP::LUNTIL:
	case logic::LOP::LDUNTIL:
		return std::make_unique<automata::ata::ResetClockFormula<logic::MTLFormula<ActionType>>>(
		  // TODO insert proper location formula
		  std::make_unique<automata::ata::LocationFormula<logic::MTLFormula<ActionType>>>(
		    logic::AtomicProposition<ActionType>{"formula"}));
	case logic::LOP::LAND:
		return std::make_unique<automata::ata::ConjunctionFormula<logic::MTLFormula<ActionType>>>(
		  init(formula.get_operands().front(), ap), init(formula.get_operands().back(), ap));
	case logic::LOP::LOR:
		return std::make_unique<automata::ata::DisjunctionFormula<logic::MTLFormula<ActionType>>>(
		  init(formula.get_operands().front(), ap), init(formula.get_operands().back(), ap));
	case logic::LOP::AP:
		if (formula == ap) {
			return std::make_unique<automata::ata::TrueFormula<logic::MTLFormula<ActionType>>>();
		} else {
			return std::make_unique<automata::ata::FalseFormula<logic::MTLFormula<ActionType>>>();
		}
	case logic::LOP::LNEG:
		if (formula.get_operands().front() == ap) {
			return std::make_unique<automata::ata::FalseFormula<logic::MTLFormula<ActionType>>>();
		} else {
			return std::make_unique<automata::ata::TrueFormula<logic::MTLFormula<ActionType>>>();
		}
	}

	return std::make_unique<automata::ata::TrueFormula<logic::MTLFormula<ActionType>>>();
}

automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ActionType>>
translate(const logic::MTLFormula<ActionType> &input_formula)
{
	auto formula  = input_formula.to_positive_normal_form();
	auto alphabet = formula.get_alphabet();
	if (alphabet.count({"phi_i"}) > 0) {
		// TODO throw exception
		assert(false);
	}
	auto locations   = get_closure(formula);
	auto untils      = formula.get_subformulas_of_type(logic::LOP::LUNTIL);
	auto dual_untils = formula.get_subformulas_of_type(logic::LOP::LDUNTIL);
	locations.insert({logic::AtomicProposition<ActionType>{"phi_i"}});
	auto accepting_locations = formula.get_subformulas_of_type(logic::LOP::LDUNTIL);
	std::set<automata::ata::Transition<logic::MTLFormula<ActionType>>> transitions;
	for (const auto &symbol : alphabet) {
		// TODO insert the actual formula (need templated ATAs for this)
		transitions.insert(automata::ata::Transition<logic::MTLFormula<ActionType>>(
		  logic::AtomicProposition<ActionType>{"phi_i"},
		  symbol.ap_,
		  std::make_unique<automata::ata::LocationFormula<logic::MTLFormula<ActionType>>>(
		    logic::AtomicProposition<ActionType>{"formula"})));

		for (const auto &until : untils) {
			auto transition_formula =
			  std::make_unique<automata::ata::DisjunctionFormula<logic::MTLFormula<ActionType>>>(
			    std::make_unique<automata::ata::ConjunctionFormula<logic::MTLFormula<ActionType>>>(
			      init(until.get_operands().back(), symbol), create_contains(until.get_interval())),
			    std::make_unique<automata::ata::ConjunctionFormula<logic::MTLFormula<ActionType>>>(
			      init(until.get_operands().front(), symbol),
			      // TODO insert proper location formula
			      std::make_unique<automata::ata::LocationFormula<logic::MTLFormula<ActionType>>>(
			        logic::AtomicProposition<ActionType>{"until"})));
			transitions.insert(
			  // TODO insert proper location formula
			  automata::ata::Transition<logic::MTLFormula<ActionType>>(
			    logic::AtomicProposition<ActionType>{"until"}, "symbol", std::move(transition_formula)));
		}
		for (const auto &dual_until : dual_untils) {
			std::make_unique<automata::ata::DisjunctionFormula<logic::MTLFormula<ActionType>>>(
			  std::make_unique<automata::ata::ConjunctionFormula<logic::MTLFormula<ActionType>>>(
			    init(dual_until.get_operands().back(), symbol),
			    create_negated_contains(dual_until.get_interval())),
			  std::make_unique<automata::ata::ConjunctionFormula<logic::MTLFormula<ActionType>>>(
			    init(dual_until.get_operands().front(), symbol),
			    // TODO insert proper location formula
			    std::make_unique<automata::ata::LocationFormula<logic::MTLFormula<ActionType>>>(
			      logic::AtomicProposition<ActionType>{"dual_until"})));
		}
	}
	return automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ActionType>>(
	  {"a"}, // TODO get alphabet from formula, needs templated ATAs
	  logic::AtomicProposition<ActionType>{"phi_i"},
	  {logic::AtomicProposition<ActionType>{""}}, // convert accepting_locations,
	  std::move(transitions));
}
} // namespace mtl_ata_translation
