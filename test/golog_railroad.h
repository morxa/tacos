/***************************************************************************
 *  golog_railroad.h - Case study generator for Railraod scenario with Golog
 *
 *  Created:   Fri 28 Jan 11:18:01 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#pragma once

#include "gocos/golog_program.h"
#include "mtl/MTLFormula.h"
#include "utilities/types.h"

#include <set>
#include <string>
#include <tuple>
#include <vector>

std::tuple<std::string,
           tacos::logic::MTLFormula<std::string>,
           std::set<std::string>,
           std::set<std::string>>
create_crossing_problem(const std::vector<tacos::Time> &distances);
