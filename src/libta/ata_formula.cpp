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

#include <libta/ata_formula.h>
#include <libta/automata.h>

namespace automata {
namespace ata {

bool
TrueFormula::is_satisfied(const std::set<State> &, const ClockValuation &) const
{
	return true;
}

bool
FalseFormula::is_satisfied(const std::set<State> &, const ClockValuation &) const
{
	return false;
}

LocationFormula::LocationFormula(const Location &location) : location_(location)
{
}

bool
LocationFormula::is_satisfied(const std::set<State> &states, const ClockValuation &v) const
{
	return states.count(std::make_pair(location_, v));
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

ResetClockFormula::ResetClockFormula(std::unique_ptr<Formula> sub_formula)
: sub_formula_(std::move(sub_formula))
{
}

bool
ResetClockFormula::is_satisfied(const std::set<State> &states, const ClockValuation &) const
{
	return sub_formula_->is_satisfied(states, 0);
}

} // namespace ata
} // namespace automata
