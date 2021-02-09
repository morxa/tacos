/***************************************************************************
 *  automata.cpp - Generic automata definitions
 *
 *  Created: Thu 28 May 2020 15:46:12 CEST 15:46
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

#include "automata/automata.h"

namespace automata {

bool
is_satisfied(const ClockConstraint &constraint, const Time &valuation)
{
	return std::visit([&](auto &&c) { return c.is_satisfied(valuation); }, constraint);
}

std::ostream &
operator<<(std::ostream &os, const ClockConstraint &constraint)
{
	std::visit([&os](auto &&c) { os << c; }, constraint);
	return os;
}

std::ostream &
operator<<(std::ostream &os, const std::multimap<std::string, const ClockConstraint> &constraints)
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

ClockSetValuation
get_valuations(const std::map<std::string, Clock> &clocks)
{
	ClockSetValuation res;
	std::transform(clocks.begin(),
	               clocks.end(),
	               std::inserter(res, res.end()),
	               [&](const std::pair<std::string, Clock> &clock) {
		               return std::make_pair(clock.first, clock.second.get_valuation());
	               });
	return res;
}

std::map<std::string, Clock>
create_clocks(const ClockSetValuation &clock_valuations)
{
	std::map<std::string, Clock> clocks;
	for (const auto &[name, value] : clock_valuations) {
		clocks[name] = Clock(value);
	}
	return clocks;
}
} // namespace automata
