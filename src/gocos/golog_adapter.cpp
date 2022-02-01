/***************************************************************************
 *  golog_adapter.cpp - Generate successors of Golog configurations
 *
 *  Created:   Fri 24 Sep 16:32:37 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "gocos/golog_adapter.h"

namespace tacos::search {

std::ostream &
operator<<(std::ostream &os, const GologLocation &location)
{
	os << "(";
	if (location.remaining_program) {
		os << gologpp::ReadylogContext::instance().to_string(*location.remaining_program);
	} else {
		os << "[]";
	}
	os << ", ";
	os << location.history->special_semantics() << ")";
	return os;
}

bool
operator<(const GologLocation &first, const GologLocation &second)
{
	if (*first.remaining_program < *second.remaining_program) {
		return true;
	}
	if (*second.remaining_program < *first.remaining_program) {
		return false;
	}
	return first.program->get_satisfied_fluents(*first.history)
	       < second.program->get_satisfied_fluents(*second.history);
}

namespace details {
std::map<std::string, double>
get_clock_values(const ClockSetValuation &clock_valuations)
{
	std::map<std::string, double> res;
	for (const auto &clock : clock_valuations) {
		res[clock.first] = clock.second.get_valuation();
	}
	return res;
}
} // namespace details
} // namespace tacos::search
