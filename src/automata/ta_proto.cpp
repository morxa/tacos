/***************************************************************************
 *  ta_proto.cpp - Protobuf import/export for timed automata
 *
 *  Created:   Fri 19 Mar 10:50:08 CET 2021
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

#include "automata/ta_proto.h"

#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta.pb.h"

#include <algorithm>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
#include <stdexcept>

namespace automata::ta {

namespace {
Transition<std::string, std::string>
parse_transition(const proto::TimedAutomaton::Transition &transition_proto)
{
	using ProtoClockConstraint = proto::TimedAutomaton::Transition::ClockConstraint;
	std::multimap<std::string, ClockConstraint> clock_constraints;
	for (const auto &clock_constraint : transition_proto.clock_constraints()) {
		switch (clock_constraint.operand()) {
		case ProtoClockConstraint::LESS:
			clock_constraints.insert(
			  {clock_constraint.clock(),
			   AtomicClockConstraintT<std::less<Time>>{clock_constraint.comparand()}});
			break;
		case ProtoClockConstraint::LESS_EQUAL:
			clock_constraints.insert(
			  {clock_constraint.clock(),
			   AtomicClockConstraintT<std::less_equal<Time>>{clock_constraint.comparand()}});
			break;
		case ProtoClockConstraint::EQUAL_TO:
			clock_constraints.insert(
			  {clock_constraint.clock(),
			   AtomicClockConstraintT<std::equal_to<Time>>{clock_constraint.comparand()}});
			break;
		case ProtoClockConstraint::NOT_EQUAL_TO:
			clock_constraints.insert(
			  {clock_constraint.clock(),
			   AtomicClockConstraintT<std::not_equal_to<Time>>{clock_constraint.comparand()}});
			break;
		case ProtoClockConstraint::GREATER_EQUAL:
			clock_constraints.insert(
			  {clock_constraint.clock(),
			   AtomicClockConstraintT<std::greater_equal<Time>>{clock_constraint.comparand()}});
			break;
		case ProtoClockConstraint::GREATER:
			clock_constraints.insert(
			  {clock_constraint.clock(),
			   AtomicClockConstraintT<std::greater<Time>>{clock_constraint.comparand()}});
			break;
		default:
			throw std::invalid_argument("Unknown clock constraint operand "
			                            + ProtoClockConstraint::Operand_Name(clock_constraint.operand()));
		}
	}
	return Transition<std::string, std::string>{
	  transition_proto.source(),
	  transition_proto.symbol(),
	  transition_proto.target(),
	  clock_constraints,
	  std::set<std::string>{begin(transition_proto.clock_resets()),
	                        end(transition_proto.clock_resets())}};
}
} // namespace

TimedAutomaton<std::string, std::string>
parse_proto(const proto::TimedAutomaton &ta_proto)
{
	return TimedAutomaton<std::string, std::string>{
	  std::set<std::string>{begin(ta_proto.locations()), end(ta_proto.locations())},
	  std::set<std::string>{begin(ta_proto.alphabet()), end(ta_proto.alphabet())},
	  ta_proto.initial_location(),
	  std::set<std::string>{begin(ta_proto.final_locations()), end(ta_proto.final_locations())},
	  std::set<std::string>{begin(ta_proto.clocks()), end(ta_proto.clocks())},
	  // Construct a Transition for each transition in the proto.
	  ranges::subrange(std::begin(ta_proto.transitions()), std::end(ta_proto.transitions()))
	    | ranges::views::transform(
	      [](const auto &transition) { return parse_transition(transition); })
	    | ranges::to_vector};
}

} // namespace automata::ta
