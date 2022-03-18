/***************************************************************************
 *  fischer.h - Utility functions for fischer test scenario
 *
 *  Created: Wed 21 Apr 2021 15:07:42 CEST 14:00
 *  Copyright  2021  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
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

#include <set>
#include <string>
#include <tuple>
#include <vector>

std::tuple<tacos::automata::ta::TimedAutomaton<std::vector<std::string>, std::string>,
           std::set<std::string>,
           std::set<std::string>>
create_fischer_instance(std::size_t     count,
                        tacos::Endpoint delay_self_assign,
                        tacos::Endpoint delay_enter_critical);
