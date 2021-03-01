/***************************************************************************
 *  ta_product.h - Compute the product automaton of timed automata
 *
 *  Created:   Mon  1 Mar 12:49:54 CET 2021
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

#ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_PRODUCT_H_
#define SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_PRODUCT_H_

#include "ta.h"

#include <iostream>
#include <tuple>

namespace automata::ta {

/** Compute the product automaton of two timed automata.
 * The resulting automaton's location set is the cartesian product of the
 * input automata's locations.
 * The product automaton either takes a single transition in either one of the automata for a
 * non-synchronized action, or it simultaneously takes a transition in both automata for a
 * synchronized action:
 * 1. For every action a in H, (l1, l2) -- (a, G1 U G2, Y1 U G2) --> (l1', l2') if
 *    a. l1 -- (a, G1, Y1) -> l1'
 *    b. l2 -- (a, G2, Y2) -> l2'
 * 2. For every action a not in H (l1, l2) -- (a, G1 U G2, Y1 U G2) --> (l1', l2') if either
 *    a. l1 -- (a, G1, Y1) -> l1' and l2' = l2, or
 *    b. l2 -- (a, G2, Y2) -> l2' and l1' = l1
 *
 * @param ta1 The first timed automaton
 * @param ta2 The second timed automaton
 * @param synchronized_actions The actions on which the two TAs must synchronize
 * @return The product automaton
 */
template <typename LocationT1, typename LocationT2, typename ActionT>
TimedAutomaton<std::tuple<LocationT1, LocationT2>, ActionT>
get_product(const TimedAutomaton<LocationT1, ActionT> &ta1,
            const TimedAutomaton<LocationT2, ActionT> &ta2,
            const std::set<ActionT>                    synchronized_actions = {});

} // namespace automata::ta

#include "ta_product.hpp"

#endif /* ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_PRODUCT_H_ */
