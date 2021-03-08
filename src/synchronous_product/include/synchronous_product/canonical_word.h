/***************************************************************************
 *  canonical_word.h - Canonical word representation
 *
 *  Created:   Fri  5 Mar 16:38:43 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#include "automata/ata.h"
#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_regions.h"
#include "mtl/MTLFormula.h"
#include "utilities/numbers.h"

/** Get the regionalized synchronous product of a TA and an ATA. */
namespace synchronous_product {

using automata::ClockValuation;
using automata::Time;
using automata::ta::RegionIndex;
template <typename Location>
/** Short-hand type alias for a configuration of a TA */
using TAConfiguration = automata::ta::Configuration<Location>;
/** Always use ATA configurations over MTLFormulas */
template <typename ActionType>
using ATAConfiguration = automata::ata::Configuration<logic::MTLFormula<ActionType>>;
/** An expanded state (location, clock_name, clock_valuation) of a TimedAutomaton */
template <typename Location>
using TAState = std::tuple<Location, std::string, ClockValuation>;
/** Always use ATA states over MTLFormulas */
template <typename ActionType>
using ATAState = automata::ata::State<logic::MTLFormula<ActionType>>;
/** An ABSymbol is either a TAState or an ATAState */
template <typename Location, typename ActionType>
using ABSymbol = std::variant<TAState<Location>, ATAState<ActionType>>;
/** A TARegionState is a tuple (location, clock_name, clock_region) */
template <typename Location>
using TARegionState = std::tuple<Location, std::string, RegionIndex>;
/** An ATARegionState is a pair (formula, clock_region) */
template <typename ActionType>
using ATARegionState = std::pair<logic::MTLFormula<ActionType>, RegionIndex>;
/** An ABRegionSymbol is either a TARegionState or an ATARegionState */
template <typename Location, typename ActionType>
using ABRegionSymbol = std::variant<TARegionState<Location>, ATARegionState<ActionType>>;

/** A canonical word H(s) for a regionalized A/B configuration */
template <typename Location, typename ActionType>
using CanonicalABWord = std::vector<std::set<ABRegionSymbol<Location, ActionType>>>;

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

template <typename Location, typename ActionType>
std::ostream &
operator<<(std::ostream &                                                                 os,
           const std::vector<synchronous_product::CanonicalABWord<Location, ActionType>> &ab_words)
{
	if (ab_words.empty()) {
		os << "{}";
		return os;
	}
	os << "{ ";
	bool first = true;
	for (const auto &ab_word : ab_words) {
		if (!first) {
			os << ", ";
		} else {
			first = false;
		}
		os << ab_word;
	}
	os << " }";
	return os;
}

template <typename Location, typename ActionType>
std::ostream &
operator<<(
  std::ostream &                                                                           os,
  const std::pair<ActionType, synchronous_product::CanonicalABWord<Location, ActionType>> &ab_word)
{
	os << "(" << ab_word.first << ", " << ab_word.second << ")";
	return os;
}

template <typename Location, typename ActionType>
std::ostream &
operator<<(
  std::ostream &os,
  const std::vector<
    std::pair<ActionType, synchronous_product::CanonicalABWord<Location, ActionType>>> &ab_words)
{
	if (ab_words.empty()) {
		os << "{}";
		return os;
	}
	os << "{ ";
	bool first = true;
	for (const auto &ab_word : ab_words) {
		if (!first) {
			os << ", ";
		} else {
			first = false;
		}
		os << ab_word;
	}
	os << " }";
	return os;
}

template <typename Location, typename ActionType>
std::ostream &
operator<<(std::ostream &                                                              os,
           const std::set<synchronous_product::CanonicalABWord<Location, ActionType>> &ab_words)
{
	if (ab_words.empty()) {
		os << "{}";
		return os;
	}
	os << "{ ";
	bool first = true;
	for (const auto &ab_word : ab_words) {
		if (!first) {
			os << ", ";
		} else {
			first = false;
		}
		os << ab_word;
	}
	os << " }";
	return os;
}
