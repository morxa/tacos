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
#include "utilities/types.h"

/** Get the regionalized synchronous product of a TA and an ATA. */
namespace tacos::search {

/** Always use ATA configurations over MTLFormulas */
template <typename ConstraintSymbolType>
using ATAConfiguration = automata::ata::Configuration<logic::MTLFormula<ConstraintSymbolType>>;

/** An expanded state (location, clock_name, clock_valuation) of a plant. */
template <typename LocationT>
struct PlantState
{
	/** The location part of this state */
	LocationT location;
	/** The clock name of this state */
	std::string clock;
	/** The clock valuation of the clock in this state */
	ClockValuation clock_valuation;
};

/** Compare two PlantStates
 * @param s1 The first state
 * @param s2 The second state
 * @return true if s1 is lexicographically smaller than s2
 */
template <typename LocationT>
bool
operator<(const PlantState<LocationT> &s1, const PlantState<LocationT> &s2)
{
	return std::tie(s1.location, s1.clock, s1.clock_valuation)
	       < std::tie(s2.location, s2.clock, s2.clock_valuation);
}

/** Always use ATA states over MTLFormulas */
template <typename ConstraintSymbolType>
using ATAState = automata::ata::State<logic::MTLFormula<ConstraintSymbolType>>;
/** An ABSymbol is either a PlantState or an ATAState */
template <typename LocationT, typename ConstraintSymbolType>
using ABSymbol = std::variant<PlantState<LocationT>, ATAState<ConstraintSymbolType>>;

/** A PlantRegionState is a tuple (location, clock_name, clock_region) */
template <typename LocationT>
struct PlantRegionState
{
	/** The location of the plant region state */
	LocationT location;
	/** The clock name of this region state */
	std::string clock;
	/** The region index (regionalized clock valuation) of the clock in this state */
	RegionIndex region_index;
};

/** Compare two plant region states.
 * @param s1 The first state
 * @param s2 The second state
 * @return true if s1 is lexicographically smaller than s2
 */
template <typename LocationT>
bool
operator<(const PlantRegionState<LocationT> &s1, const PlantRegionState<LocationT> &s2)
{
	return std::tie(s1.location, s1.clock, s1.region_index)
	       < std::tie(s2.location, s2.clock, s2.region_index);
}

/** Check two plant region states for equality.
 * Two plant region states are considered equal if they have the same location, clock name, and
 * region index.
 * @param s1 The first state
 * @param s2 The second state
 * @return true if s1 is equal to s2
 */
template <typename LocationT>
bool
operator==(const PlantRegionState<LocationT> &s1, const PlantRegionState<LocationT> &s2)
{
	return !(s1 < s2) && !(s2 < s1);
}

/** An ATARegionState is a pair (formula, clock_region) */
template <typename ConstraintSymbolType>
struct ATARegionState
{
	/** The ATA formula in the regionalized ATA state */
	logic::MTLFormula<ConstraintSymbolType> formula;
	/** The region index of the state */
	RegionIndex region_index;
};

/** Compare two ATA region states.
 * @param s1 The first state
 * @param s2 The second state
 * @return true if s1 is lexicographically smaller than s2
 */
template <typename LocationT>
bool
operator<(const ATARegionState<LocationT> &s1, const ATARegionState<LocationT> &s2)
{
	return std::tie(s1.formula, s1.region_index) < std::tie(s2.formula, s2.region_index);
}

/** Check two ATA region states for equality.
 * Two ATA region states are considered equal if they have the same location and region
 * index.
 * @param s1 The first state
 * @param s2 The second state
 * @return true if s1 is equal to s2
 */
template <typename LocationT>
bool
operator==(const ATARegionState<LocationT> &s1, const ATARegionState<LocationT> &s2)
{
	return !(s1 < s2) && !(s2 < s1);
}

/** An ABRegionSymbol is either a TARegionState or an ATARegionState */
template <typename LocationT, typename ConstraintSymbolType>
using ABRegionSymbol =
  std::variant<PlantRegionState<LocationT>, ATARegionState<ConstraintSymbolType>>;

/** A canonical word H(s) for a regionalized A/B configuration */
template <typename LocationT, typename ConstraintSymbolT>
using CanonicalABWord = std::vector<std::set<ABRegionSymbol<LocationT, ConstraintSymbolT>>>;

/** Print a PlantRegionState. */
template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const search::PlantRegionState<LocationT> &state)
{
	os << "(" << state.location << ", " << state.clock << ", " << state.region_index << ")";
	return os;
}

/** Print an ATARegionState. */
template <typename ConstraintSymbolType>
std::ostream &
operator<<(std::ostream &os, const search::ATARegionState<ConstraintSymbolType> &state)
{
	os << "(" << state.formula << ", " << state.region_index << ")";
	return os;
}

/** Print an ABRegionSymbol. */
template <typename LocationT, typename ConstraintSymbolType>
std::ostream &
operator<<(std::ostream &os, const search::ABRegionSymbol<LocationT, ConstraintSymbolType> &symbol)
{
	std::visit([&os](const auto &v) { os << v; }, symbol);
	return os;
}

/** Print a set of ABRegionSymbols (a letter of a CanonicalABWord). */
template <typename LocationT, typename ConstraintSymbolType>
std::ostream &
operator<<(std::ostream &                                                           os,
           const std::set<search::ABRegionSymbol<LocationT, ConstraintSymbolType>> &word)
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

/** Print a CanonicalABWord. */
template <typename LocationT, typename ConstraintSymbolType>
std::ostream &
operator<<(
  std::ostream &                                                                        os,
  const std::vector<std::set<search::ABRegionSymbol<LocationT, ConstraintSymbolType>>> &word)
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

/** Print a vector of CanonicalABWords. */
template <typename LocationT, typename ConstraintSymbolType>
std::ostream &
operator<<(std::ostream &                                                               os,
           const std::vector<search::CanonicalABWord<LocationT, ConstraintSymbolType>> &ab_words)
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

/** Print a next canonical word along with its region index and action. */
template <typename LocationT, typename ActionType, typename ConstraintSymbolType>
std::ostream &
operator<<(std::ostream &                                                              os,
           const std::tuple<RegionIndex,
                            ActionType,
                            search::CanonicalABWord<LocationT, ConstraintSymbolType>> &ab_word)
{
	os << "(" << std::get<0>(ab_word) << ", " << std::get<1>(ab_word) << ", " << std::get<2>(ab_word)
	   << ")";
	return os;
}

/** Print a vector of next canonical words. */
template <typename LocationT, typename ActionType, typename ConstraintSymbolType>
std::ostream &
operator<<(
  std::ostream &os,
  const std::vector<
    std::tuple<RegionIndex, ActionType, search::CanonicalABWord<LocationT, ConstraintSymbolType>>>
    &ab_words)
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

/** Print a set of CanonicalABWords. */
template <typename LocationT, typename ConstraintSymbolType>
std::ostream &
operator<<(std::ostream &                                                            os,
           const std::set<search::CanonicalABWord<LocationT, ConstraintSymbolType>> &ab_words)
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

} // namespace tacos::search
