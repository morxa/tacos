/***************************************************************************
 *  fischer.h - Utility functions for fischer test scenario
 *
 *  Created: Wed 21 Apr 2021 15:07:42 CEST 14:00
 *  Copyright  2021  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



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
create_fischer_instance(std::size_t               count,
                        tacos::automata::Endpoint delay_self_assign,
                        tacos::automata::Endpoint delay_enter_critical);
