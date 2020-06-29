/***************************************************************************
 *  ata.cpp - Alternating Timed Automata
 *
 *  Created: Fri 05 Jun 2020 11:54:51 CEST 11:54
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

#include <ta/ata.h>
#include <ta/automata.h>

#include <cassert>
#include <iterator>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/cartesian_product.hpp>
#include <variant>

namespace automata::ata {

template class AlternatingTimedAutomaton<std::string, std::string>;

} // namespace automata::ata
