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

#include "automata/ta.h"

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
#include <stdexcept>

namespace automata::ta {

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
	    | ranges::views::transform([](const auto &transition) {
		      return Transition<std::string, std::string>{transition.source(),
		                                                  transition.symbol(),
		                                                  transition.target()};
	      })
	    | ranges::to_vector};
}

} // namespace automata::ta
