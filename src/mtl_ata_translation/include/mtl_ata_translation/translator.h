/***************************************************************************
 *  translator.h - Translate an MTL formula into an ATA
 *
 *  Created: Thu 18 Jun 2020 11:06:49 CEST 11:06
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#ifndef SRC_MTL_ATA_TRANSLATION_INCLUDE_MTL_ATA_TRANSLATION_TRANSLATOR_H
#define SRC_MTL_ATA_TRANSLATION_INCLUDE_MTL_ATA_TRANSLATION_TRANSLATOR_H

#include "automata/ata.h"
#include "mtl/MTLFormula.h"

/// Translate an MTL formula into an ATA.
namespace tacos::mtl_ata_translation {

/** Translate an MTL formula into an ATA.
 * Create the ATA closely following the construction by Ouaknine and Worrell, 2005.
 * @param input_formula The formula to translate
 * @param input_alphabet The alphabet that the ATA should read, defaults to the symbols of the
 * formula.
 * @return An ATA that accepts a word w iff the word is in the language of the formula.
 */
template <typename ConstraintSymbolT,
          typename SymbolT = ConstraintSymbolT,
          bool state_based = false>
automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ConstraintSymbolT>,
                                         logic::AtomicProposition<SymbolT>>
translate(const logic::MTLFormula<ConstraintSymbolT> &input_formula,
          std::set<logic::AtomicProposition<SymbolT>> input_alphabet = {});

} // namespace tacos::mtl_ata_translation

#include "translator.hpp"

#endif /* ifndef SRC_MTL_ATA_TRANSLATION_INCLUDE_MTL_ATA_TRANSLATION_TRANSLATOR_H */
