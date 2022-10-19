/***************************************************************************
 *  golog_household.h - Case study generator for a household robot
 *
 *  Created:   Wed 19 Oct 19:43:03 CEST 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include "mtl/MTLFormula.h"

#include <set>
#include <string>
#include <tuple>

std::tuple<std::string,
           tacos::logic::MTLFormula<std::string>,
           std::set<std::string>,
           std::set<std::string>>
create_household_problem();
