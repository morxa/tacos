/***************************************************************************
 *  golog_symbols.cpp - Utility functions to deal with Golog symbols
 *
 *  Created:   Tue 25 Jan 14:45:26 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "gocos/golog_symbols.h"

#include <regex>
#include <stdexcept>

namespace tacos::search {

std::pair<std::string, std::vector<std::string>>
split_symbol(const std::string &symbol)
{
	const std::regex name_regex("\\s*(\\w+)\\s*(?:\\(\\s*(.*?)\\s*\\))?\\s*");
	const std::regex split_args_regex("[^\\s,]+");
	std::smatch      matches;
	std::regex_match(symbol, matches, name_regex);
	if (matches.size() != 2 && matches.size() != 3) {
		throw std::invalid_argument("Unexpected regex match");
	}
	const std::string args_string = matches[2];
	if (matches.size() == 2) {
		return {matches[1], {}};
	}
	std::vector<std::string> args;
	for (auto split_it =
	       std::regex_iterator(std::begin(args_string), std::end(args_string), split_args_regex);
	     split_it != std::sregex_iterator();
	     split_it++) {
		args.push_back(split_it->str());
	}
	return {matches[1], args};
}

} // namespace tacos::search
