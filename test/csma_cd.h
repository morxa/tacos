/***************************************************************************
 *  csma_cd.h - Utility functions for csma_cd test scenario
 *
 *  Created: Thu 22 Apr 2021 19:33:42 CEST 14:00
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
create_csma_cd_instance(std::size_t     count,
                        tacos::Endpoint delay_self_assign,
                        tacos::Endpoint delay_enter_critical);
