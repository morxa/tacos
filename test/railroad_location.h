/***************************************************************************
 *  railroad.h - Utility functions for railroad test scenario
 *
 *  Created: Tue 20 Apr 2021 13:00:42 CEST 13:00
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
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#pragma once

#include "automata/automata.h"
#include "automata/ta.h"
#include "mtl/MTLFormula.h"

#include <set>
#include <string>
#include <tuple>
#include <vector>

std::tuple<tacos::automata::ta::TimedAutomaton<std::vector<std::string>, std::string>,
           tacos::logic::MTLFormula<
             tacos::automata::ta::TimedAutomaton<std::vector<std::string>, std::string>::Location>,
           std::set<std::string>,
           std::set<std::string>>
create_crossing_problem(std::vector<tacos::Time> distances);
