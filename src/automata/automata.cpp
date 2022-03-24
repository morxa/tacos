/***************************************************************************
 *  automata.cpp - Generic automata definitions
 *
 *  Created: Thu 28 May 2020 15:46:12 CEST 15:46
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



#include "automata/automata.h"

namespace tacos::automata {

bool
is_satisfied(const ClockConstraint &constraint, const Time &valuation)
{
	return std::visit([&](auto &&c) { return c.is_satisfied(valuation); }, constraint);
}

std::ostream &
operator<<(std::ostream &os, const automata::ClockConstraint &constraint)
{
	std::visit([&os](auto &&c) { os << c; }, constraint);
	return os;
}

std::ostream &
operator<<(std::ostream &                                               os,
           const std::multimap<std::string, automata::ClockConstraint> &constraints)
{
	if (constraints.empty()) {
		os << u8"⊤";
		return os;
	}
	bool first = true;
	for (const auto &[clock, constraint] : constraints) {
		if (first) {
			first = false;
		} else {
			os << u8" ∧ ";
		}
		os << clock << " " << constraint;
	}
	return os;
}

} // namespace tacos::automata
