/***************************************************************************
 *  translator.h - Translate an MTL formula into an ATA
 *
 *  Created: Thu 18 Jun 2020 11:06:49 CEST 11:06
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

#pragma once

#include "mtl/MTLFormula.h"
// MTL needs to be included first to have operator<< available for MTLFormula.
#include "automata/ata.h"

/// Translate an MTL formula into an ATA.
namespace mtl_ata_translation {

// TODO We should deduce the ActionType from the MTLFormula template type.
/// The type of the MTL formula symbols.
using ActionType = std::string;

/** Translate an MTL formula into an ATA.
 * Create the ATA closely following the construction by Ouaknine and Worrell, 2005.
 * @param input_formula The formula to translate
 * @param alphabet The alphabet that the ATA should read, defaults to the symbols of the formula.
 * @return An ATA that accepts a word w iff the word is in the language of the formula.
 */
automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ActionType>,
                                         logic::AtomicProposition<ActionType>>
translate(const logic::MTLFormula<ActionType> &          input_formula,
          std::set<logic::AtomicProposition<ActionType>> alphabet = {});

} // namespace mtl_ata_translation
