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
	  Location<std::string>{transition_proto.source()},
	  transition_proto.symbol(),
	  Location<std::string>{transition_proto.target()},
	  clock_constraints,
	  std::set<std::string>{begin(transition_proto.clock_resets()),
	                        end(transition_proto.clock_resets())}};
}

proto::TimedAutomaton::Transition::ClockConstraint
clock_constraint_to_proto(const std::string &clock_name, const ClockConstraint &constraint)
{
	proto::TimedAutomaton::Transition::ClockConstraint proto;
	proto.set_clock(clock_name);
	std::visit(
	  [&proto](const auto &c) {
		  proto.set_comparand(c.get_comparand());
		  using T                    = std::decay_t<decltype(c)>;
		  using ProtoClockConstraint = proto::TimedAutomaton::Transition::ClockConstraint;
		  if constexpr (std::is_same_v<T, AtomicClockConstraintT<std::less<Time>>>) {
			  proto.set_operand(ProtoClockConstraint::LESS);
		  } else if constexpr (std::is_same_v<T, AtomicClockConstraintT<std::less_equal<Time>>>) {
			  proto.set_operand(ProtoClockConstraint::LESS_EQUAL);
		  } else if constexpr (std::is_same_v<T, AtomicClockConstraintT<std::equal_to<Time>>>) {
			  proto.set_operand(ProtoClockConstraint::EQUAL_TO);
		  } else if constexpr (std::is_same_v<T, AtomicClockConstraintT<std::greater_equal<Time>>>) {
			  proto.set_operand(ProtoClockConstraint::GREATER_EQUAL);
		  } else if constexpr (std::is_same_v<T, AtomicClockConstraintT<std::greater<Time>>>) {
			  proto.set_operand(ProtoClockConstraint::GREATER);
		  } else {
			  std::logic_error("Unexpected constraint type!");
		  }
	  },
	  constraint);
	return proto;
}

proto::TimedAutomaton::Transition
transition_to_proto(const Transition<std::string, std::string> &transition)
{
	proto::TimedAutomaton::Transition proto;
	proto.set_source(transition.source_.get());
	proto.set_symbol(transition.symbol_);
	proto.set_target(transition.target_.get());
	*proto.mutable_clock_resets() = {std::begin(transition.clock_resets_),
	                                 std::end(transition.clock_resets_)};
	for (const auto &[clock, constraint] : transition.clock_constraints_) {
		proto.mutable_clock_constraints()->Add(clock_constraint_to_proto(clock, constraint));
	}
	return proto;
}
} // namespace

TimedAutomaton<std::string, std::string>
parse_proto(const proto::TimedAutomaton &ta_proto)
{
	return TimedAutomaton<std::string, std::string>{
	  ranges::subrange(std::begin(ta_proto.locations()), std::end(ta_proto.locations()))
	    | ranges::views::transform([](const auto &name) { return Location<std::string>{name}; })
	    | ranges::to<std::set>,
	  std::set<std::string>{begin(ta_proto.alphabet()), end(ta_proto.alphabet())},
	  Location<std::string>{ta_proto.initial_location()},
	  ranges::subrange(std::begin(ta_proto.final_locations()), std::end(ta_proto.final_locations()))
	    | ranges::views::transform([](const auto &name) { return Location<std::string>{name}; })
	    | ranges::to<std::set>,
	  std::set<std::string>{begin(ta_proto.clocks()), end(ta_proto.clocks())},
	  // Construct a Transition for each transition in the proto.
	  ranges::subrange(std::begin(ta_proto.transitions()), std::end(ta_proto.transitions()))
	    | ranges::views::transform(
	      [](const auto &transition) { return parse_transition(transition); })
	    | ranges::to_vector};
}

proto::TimedAutomaton
ta_to_proto(const TimedAutomaton<std::string, std::string> &ta)
{
	proto::TimedAutomaton proto;
	for (const auto &location : ta.get_locations()) {
		proto.mutable_locations()->Add(std::string{location.get()});
	}
	for (const auto &location : ta.get_final_locations()) {
		proto.mutable_final_locations()->Add(std::string{location.get()});
	}
	proto.set_initial_location(ta.get_initial_location());
	*proto.mutable_alphabet() = {std::begin(ta.get_alphabet()), std::end(ta.get_alphabet())};
	*proto.mutable_clocks()   = {std::begin(ta.get_clocks()), std::end(ta.get_clocks())};
	for (const auto &[source, transition] : ta.get_transitions()) {
		proto.mutable_transitions()->Add(transition_to_proto(transition));
	}
	return proto;
}

} // namespace automata::ta
