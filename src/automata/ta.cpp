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

namespace automata {
namespace ta {

template class TimedAutomaton<std::string>;

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

} // namespace ta
} // namespace automata
