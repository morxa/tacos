/***************************************************************************
 *  automata.hpp - Generic automata definitions
 *
 *  Created:   Fri 19 Feb 13:30:37 CET 2021
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

#include "automata.h"

namespace tacos::automata {

template <class Comp>
std::ostream &
operator<<(std::ostream &os, const automata::AtomicClockConstraintT<Comp> &constraint)
{
	using automata::InvalidClockComparisonOperatorException;

	if constexpr (std::is_same_v<Comp, std::less<Time>>) {
		os << "<";
	} else if constexpr (std::is_same_v<Comp, std::less_equal<Time>>) {
		os << u8"≤";
	} else if constexpr (std::is_same_v<Comp, std::equal_to<Time>>) {
		os << "=";
	} else if constexpr (std::is_same_v<Comp, std::not_equal_to<Time>>) {
		os << u8"≠";
	} else if constexpr (std::is_same_v<Comp, std::greater_equal<Time>>) {
		os << u8"≥";
	} else if constexpr (std::is_same_v<Comp, std::greater<Time>>) {
		os << ">";
	} else {
		throw InvalidClockComparisonOperatorException();
	}
	os << " " << constraint.comparand_;
	return os;
}

template <typename ActionT>
std::ostream &
operator<<(
  std::ostream &                                                                       os,
  const std::multimap<ActionT, std::multimap<std::string, automata::ClockConstraint>> &constraints)
{
	bool first = true;
	for (const auto &[action, action_constraints] : constraints) {
		if (first) {
			first = false;
		} else {
			os << ", ";
		}
		os << action << ": " << action_constraints;
	}
	return os;
}

} // namespace tacos::automata
