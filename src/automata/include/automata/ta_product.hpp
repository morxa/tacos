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

#pragma once

#include "automata/ta.h"
#include "ta_product.h"

#include <iterator>
#include <range/v3/view.hpp>
#include <range/v3/view/cartesian_product.hpp>

namespace automata::ta {

template <typename LocationT1, typename LocationT2, typename ActionT>
TimedAutomaton<std::tuple<LocationT1, LocationT2>, ActionT>
get_product(const TimedAutomaton<LocationT1, ActionT> &ta1,
            const TimedAutomaton<LocationT2, ActionT> &ta2,
            const std::set<ActionT>                    synchronized_actions)
{
	// TODO implement synchronized actions
	assert(synchronized_actions.empty());
	TimedAutomaton<std::tuple<LocationT1, LocationT2>, ActionT> res{
	  ranges::views::concat(ta1.get_alphabet(), ta2.get_alphabet()) | ranges::to<std::set>(),
	  std::make_tuple(ta1.get_initial_location(), ta2.get_initial_location()),
	  ranges::views::cartesian_product(ta1.get_final_locations(), ta2.get_final_locations())
	    | ranges::to<std::set>()};
	res.add_locations(ranges::views::cartesian_product(ta1.get_locations(), ta2.get_locations())
	                  | ranges::to<std::set>());
	for (const auto &[location, transition] : ta1.get_transitions()) {
		for (const auto &l2 : ta2.get_locations()) {
			res.add_transition(Transition{std::make_tuple(location, l2),
			                              transition.symbol_,
			                              std::make_tuple(transition.target_, l2),
			                              transition.clock_constraints_,
			                              transition.clock_resets_});
		}
	}
	for (const auto &[location, transition] : ta2.get_transitions()) {
		for (const auto &l1 : ta1.get_locations()) {
			res.add_transition(Transition{std::make_tuple(l1, location),
			                              transition.symbol_,
			                              std::make_tuple(l1, transition.target_),
			                              transition.clock_constraints_,
			                              transition.clock_resets_});
		}
	}
	return res;
}

} // namespace automata::ta
