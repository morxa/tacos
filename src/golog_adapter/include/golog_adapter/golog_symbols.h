/***************************************************************************
 *  golog_symbols.h - Utility functions to deal with Golog symbols
 *
 *  Created:   Tue 25 Jan 14:43:36 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace tacos::search {

/** Split a symbol into its name and its parameters, e.g., "foo(bar)" -> {"foo", {"bar"}}.
 * @param symbol The symbol to split, either in the form "foo(bar)", "foo()", or "foo".
 * @return A pair of name and a vector of args */
std::pair<std::string, std::vector<std::string>> split_symbol(const std::string &symbol);

} // namespace tacos::search
