/***************************************************************************
 *  golog_adapter.cpp - Generate successors of Golog configurations
 *
 *  Created:   Fri 24 Sep 16:32:37 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "golog_adapter/golog_adapter.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace tacos::search {

std::ostream &
operator<<(std::ostream &os, const GologLocation &location)
{
	static std::map<std::string, unsigned int> subprograms;
	os << "(";
	if (location.remaining_program) {
		const std::string program =
		  gologpp::ReadylogContext::instance().to_string(*location.remaining_program);
		auto it = subprograms.find(program);
		if (it == std::end(subprograms)) {
			unsigned int id      = subprograms.size();
			subprograms[program] = id;
			SPDLOG_INFO("New subprogram with ID {}: {}", id, program);
			os << id;
		} else {
			os << it->second;
		}
	} else {
		os << "[]";
	}
	os << ", ";
	fmt::print(os, "[{}]", fmt::join(location.satisfied_fluents, ", "));
	os << ")";
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
	return first.satisfied_fluents < second.satisfied_fluents;
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
