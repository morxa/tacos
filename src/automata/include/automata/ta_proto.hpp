/***************************************************************************
 *  ta_proto.hpp - Protobuf import/export for timed automata
 *
 *  Created:   Thu 22 Apr 08:15:44 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

#include "automata/ta.h"
#include "ta_proto.h"
#include "utilities/to_string.h"

namespace tacos::automata::ta {

namespace details {

template <typename LocationT, typename ActionT>
proto::TimedAutomaton::Transition
transition_to_proto(const Transition<LocationT, ActionT> &transition)
{
	using utilities::to_string;
	proto::TimedAutomaton::Transition proto;
	proto.set_source(to_string(transition.source_));
	proto.set_symbol(transition.symbol_);
	proto.set_target(to_string(transition.target_));
	*proto.mutable_clock_resets() = {std::begin(transition.clock_resets_),
	                                 std::end(transition.clock_resets_)};
	for (const auto &[clock, constraint] : transition.clock_constraints_) {
		proto.mutable_clock_constraints()->Add(clock_constraint_to_proto(clock, constraint));
	}
	return proto;
}

} // namespace details

template <typename LocationT, typename ActionT>
proto::TimedAutomaton
ta_to_proto(const TimedAutomaton<LocationT, ActionT> &ta)
{
	using utilities::to_string;
	proto::TimedAutomaton proto;
	for (const auto &location : ta.get_locations()) {
		proto.mutable_locations()->Add(to_string(location));
	}
	for (const auto &location : ta.get_final_locations()) {
		proto.mutable_final_locations()->Add(to_string(location));
	}
	proto.set_initial_location(to_string(ta.get_initial_location()));
	*proto.mutable_alphabet() = {std::begin(ta.get_alphabet()), std::end(ta.get_alphabet())};
	*proto.mutable_clocks()   = {std::begin(ta.get_clocks()), std::end(ta.get_clocks())};
	for (const auto &[source, transition] : ta.get_transitions()) {
		proto.mutable_transitions()->Add(details::transition_to_proto(transition));
	}
	return proto;
}

} // namespace tacos::automata::ta
