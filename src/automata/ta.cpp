/***************************************************************************
 *  ta.cpp - Core functionality for timed automata
 *
 *  Created: Tue 26 May 2020 13:44:12 CEST 13:44
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

#include "automata/ta.h"

#include "automata/ata.h"
#include "automata/ta.hpp"
#include "automata/ta_regions.h"

namespace automata {
namespace ta {

template class TimedAutomaton<std::string, std::string>;
template class TimedAutomaton<unsigned int, std::string>;
template class TimedAutomaton<int, std::string>;

template bool operator==(const Transition<std::string, std::string> &lhs,
                         const Transition<std::string, std::string> &rhs);

template std::ostream &operator<<(std::ostream &                              os,
                                  const Transition<std::string, std::string> &transition);

template std::ostream &
operator<<(std::ostream &                                                         os,
           const std::multimap<std::string, Transition<std::string, std::string>> transitions);

std::ostream &
operator<<(std::ostream &os, const std::set<std::string> &strings)
{
	if (strings.empty()) {
		os << "{}";
		return os;
	}
	os << "{ ";
	std::copy(strings.begin(), strings.end(), std::experimental::make_ostream_joiner(os, ", "));
	os << " }";
	return os;
}

} // namespace ta
} // namespace automata

template std::ostream &operator<<(std::ostream &                                  os,
                                  const automata::ta::Configuration<std::string> &configuration);
