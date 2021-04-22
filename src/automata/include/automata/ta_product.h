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

class NotImplementedException : public std::logic_error
{
	using std::logic_error::logic_error;
};

/** Print a product location. */
template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const Location<std::vector<LocationT>> &product_location)
{
	os << "(";
	bool first = true;
	for (const auto &location : product_location.get()) {
		if (first) {
			first = false;
		} else {
			os << ", ";
		}
		os << location;
	}
	os << ")";
	return os;
}

/**
 * @brief  Checks which automata may synchronize on which label by comparing the set of
 * synchronizing labels and the individual alphabets.
 * @tparam ActionT labeltype
 * @param synchronized_actions a set of actions which should be synchronized
 * @param alphabets a vector of alphabets for different ta which should be synchonized
 * @return std::map<ActionT, std::vector<std::size_t>> a map which assigns each label indices of ta
 * which synchronize with this label
 */
template <typename ActionT>
std::map<ActionT, std::vector<std::size_t>>
collect_synchronizing_alphabets(const std::set<ActionT> &             synchronized_actions,
                                const std::vector<std::set<ActionT>> &alphabets);

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
template <typename LocationT, typename ActionT>
TimedAutomaton<std::vector<LocationT>, ActionT>
get_product(const std::vector<TimedAutomaton<LocationT, ActionT>> &automata,
            const std::set<ActionT> &                              synchronized_actions = {});

} // namespace automata::ta

#include "ta_product.hpp"

#endif /* ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_PRODUCT_H_ */
