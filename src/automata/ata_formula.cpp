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

#include "automata/ata_formula.h"

#include "automata/automata.h"

#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>

namespace automata {
namespace ata {

template class Formula<std::string>;
template class TrueFormula<std::string>;
template class FalseFormula<std::string>;
template class LocationFormula<std::string>;
template class ClockConstraintFormula<std::string>;
template class ConjunctionFormula<std::string>;
template class DisjunctionFormula<std::string>;
template class ResetClockFormula<std::string>;

template class Formula<int>;
template class TrueFormula<int>;
template class FalseFormula<int>;
template class LocationFormula<int>;
template class ClockConstraintFormula<int>;
template class ConjunctionFormula<int>;
template class DisjunctionFormula<int>;
template class ResetClockFormula<int>;

template class Formula<unsigned int>;
template class TrueFormula<unsigned int>;
template class FalseFormula<unsigned int>;
template class LocationFormula<unsigned int>;
template class ClockConstraintFormula<unsigned int>;
template class ConjunctionFormula<unsigned int>;
template class DisjunctionFormula<unsigned int>;
template class ResetClockFormula<unsigned int>;

} // namespace ata
} // namespace automata

template std::ostream &operator<<(std::ostream &os, const automata::ata::State<std::string> &state);
