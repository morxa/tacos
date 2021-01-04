/***************************************************************************
 *  synchronous_product.h - The synchronous product of a TA and an ATA
 *
 *  Created:   Mon 21 Dec 16:13:49 CET 2020
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
 *  Read the full text in the LICENSE.md file.
 */

#include "ta/automata.h"
#include "ta/ta_regions.h"

#include <mtl/MTLFormula.h>
#include <ta/ata.h>
#include <ta/ta.h>
#include <utilities/numbers.h>

#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace synchronous_product {

using automata::ClockValuation;
using automata::Time;
template <typename ActionType>
using ATAConfiguration = automata::ata::Configuration<logic::MTLFormula<ActionType>>;
template <typename Location>
using TAState = std::tuple<Location, std::string, ClockValuation>;
template <typename ActionType>
using ATAState = automata::ata::State<logic::MTLFormula<ActionType>>;
template <typename Location, typename ActionType>
using ABSymbol = std::variant<TAState<Location>, ATAState<ActionType>>;
template <typename Location>
using TARegionState = std::tuple<Location, std::string, std::size_t>;
template <typename ActionType>
using ATARegionState = std::pair<logic::MTLFormula<ActionType>, std::size_t>;
template <typename Location, typename ActionType>
using ABRegionSymbol = std::variant<TARegionState<Location>, ATARegionState<ActionType>>;

template <typename Location, typename ActionType>
ClockValuation
get_time(const ABSymbol<Location, ActionType> &w)
{
	if (std::holds_alternative<TAState<Location>>(w)) {
		return std::get<2>(std::get<TAState<Location>>(w));
	} else {
		return std::get<ATAState<ActionType>>(w).second;
	}
}

template <typename Location, typename ActionType>
std::vector<std::set<ABRegionSymbol<Location, ActionType>>>
get_canonical_word(const automata::ta::Configuration<Location> &ta_configuration,
                   const ATAConfiguration<ActionType> &         ata_configuration,
                   const unsigned int                           K)
{
	std::set<ABSymbol<Location, ActionType>> g;
	// Insert ATA configurations into g.
	std::copy(ata_configuration.begin(), ata_configuration.end(), std::inserter(g, g.end()));
	// Insert TA configurations into g.
	for (const auto &[clock_name, clock_value] : ta_configuration.second) {
		g.insert(std::make_tuple(ta_configuration.first, clock_name, clock_value));
	}
	// Sort into partitions by the fractional parts.
	std::map<double, std::set<ABSymbol<Location, ActionType>>> partitioned_g;
	for (const ABSymbol<Location, ActionType> &symbol : g) {
		partitioned_g[utilities::getFractionalPart<int, ClockValuation>(get_time(symbol))].insert(
		  symbol);
	}
	// Replace exact clock values by region indices.
	automata::ta::TimedAutomatonRegions                         regionSet{K};
	std::vector<std::set<ABRegionSymbol<Location, ActionType>>> abs;
	for (const auto &[fractional_part, g_i] : partitioned_g) {
		std::set<ABRegionSymbol<Location, ActionType>> abs_i;
		std::transform(
		  g_i.begin(),
		  g_i.end(),
		  std::inserter(abs_i, abs_i.end()),
		  [&](const ABSymbol<Location, ActionType> &w) -> ABRegionSymbol<Location, ActionType> {
			  if (std::holds_alternative<TAState<Location>>(w)) {
				  const TAState<Location> &s = std::get<TAState<Location>>(w);
				  return TARegionState<Location>(std::get<0>(s),
				                                 std::get<1>(s),
				                                 regionSet.getRegionIndex(std::get<2>(s)));
			  } else {
				  const ATAState<ActionType> &s = std::get<ATAState<ActionType>>(w);
				  return ATARegionState<ActionType>(s.first, regionSet.getRegionIndex(s.second));
			  }
		  });
		abs.push_back(abs_i);
	}
	return abs;
}

} // namespace synchronous_product

template <typename Location>
std::ostream &
operator<<(std::ostream &os, const synchronous_product::TARegionState<Location> &state)
{
	os << "(" << std::get<0>(state) << ", " << std::get<1>(state) << ", " << std::get<2>(state)
	   << ")";
	return os;
}

template <typename ActionType>
std::ostream &
operator<<(std::ostream &os, const synchronous_product::ATARegionState<ActionType> &state)
{
	os << "(" << state.first << ", " << state.second << ")";
	return os;
}

template <typename Location, typename ActionType>
std::ostream &
operator<<(std::ostream &                                                   os,
           const synchronous_product::ABRegionSymbol<Location, ActionType> &symbol)
{
	std::visit([&os](const auto &v) { os << v; }, symbol);
	return os;
}

template <typename Location, typename ActionType>
std::ostream &
operator<<(std::ostream &                                                             os,
           const std::set<synchronous_product::ABRegionSymbol<Location, ActionType>> &word)
{
	if (word.empty()) {
		os << "{}";
		return os;
	}
	os << "{ ";
	bool first = true;
	for (const auto &symbol : word) {
		if (!first) {
			os << ", ";
		} else {
			first = false;
		}
		os << symbol;
	}
	os << " }";
	return os;
}

template <typename Location, typename ActionType>
std::ostream &
operator<<(
  std::ostream &                                                                          os,
  const std::vector<std::set<synchronous_product::ABRegionSymbol<Location, ActionType>>> &word)
{
	if (word.empty()) {
		os << "[]";
		return os;
	}
	os << "[ ";
	bool first = true;
	for (const auto &symbol : word) {
		if (!first) {
			os << ", ";
		} else {
			first = false;
		}
		os << symbol;
	}
	os << " ]";
	return os;
}
