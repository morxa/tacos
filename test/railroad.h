/***************************************************************************
 *  railroad.h - Utility functions for railroad test scenario
 *
 *  Created: Tue 20 Apr 2021 13:00:42 CEST 13:00
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



#pragma once

#include "automata/automata.h"
#include "automata/ta.h"
#include "mtl/MTLFormula.h"

#include <set>
#include <string>
#include <tuple>
#include <vector>

std::tuple<tacos::automata::ta::TimedAutomaton<std::vector<std::string>, std::string>,
           tacos::logic::MTLFormula<std::string>,
           std::set<std::string>,
           std::set<std::string>>
create_crossing_problem(std::vector<tacos::Endpoint> distances);
