/***************************************************************************
 *  canonical_word.h - Canonical word representation
 *
 *  Created:   Fri  5 Mar 16:38:43 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

#include "automata/ata.h"
#include "automata/automata.h"
#include "automata/ta.h"
#include "automata/ta_regions.h"
#include "mtl/MTLFormula.h"
#include "utilities/numbers.h"

/** Get the regionalized synchronous product of a TA and an ATA. */
namespace tacos::search {

using automata::ClockValuation;
using automata::Time;
using automata::ta::RegionIndex;
template <typename LocationT>
/** Short-hand type alias for a configuration of a TA */
using TAConfiguration = automata::ta::Configuration<LocationT>;
/** Always use ATA configurations over MTLFormulas */
template <typename ConstraintSymbolType>
using ATAConfiguration = automata::ata::Configuration<logic::MTLFormula<ConstraintSymbolType>>;

/** An expanded state (location, clock_name, clock_valuation) of a TimedAutomaton */
template <typename LocationT>
struct TAState
{
	/** The location part of this state */
	automata::ta::Location<LocationT> location;
	/** The clock name of this state */
	std::string clock;
	/** The clock valuation of the clock in this state */
	ClockValuation clock_valuation;
};

/** Compare two TAStates
 * @param s1 The first state
 * @param s2 The second state
 * @return true if s1 is lexicographically smaller than s2
 */
template <typename LocationT>
bool
operator<(const TAState<LocationT> &s1, const TAState<LocationT> &s2)
{
	return std::tie(s1.location, s1.clock, s1.clock_valuation)
	       < std::tie(s2.location, s2.clock, s2.clock_valuation);
}

/** Always use ATA states over MTLFormulas */
template <typename ConstraintSymbolType>
using ATAState = automata::ata::State<logic::MTLFormula<ConstraintSymbolType>>;
/** An ABSymbol is either a TAState or an ATAState */
template <typename LocationT, typename ConstraintSymbolType>
using ABSymbol = std::variant<TAState<LocationT>, ATAState<ConstraintSymbolType>>;

/** A TARegionState is a tuple (location, clock_name, clock_region) */
template <typename LocationT>
struct TARegionState
{
	/** The location of the TA region state */
	automata::ta::Location<LocationT> location;
	/** The clock name of this region state */
	std::string clock;
	/** The region index (regionalized clock valuation) of the clock in this state */
	RegionIndex region_index;
};

/** Compare two TA region states.
 * @param s1 The first state
 * @param s2 The second state
 * @return true if s1 is lexicographically smaller than s2
 */
template <typename LocationT>
bool
operator<(const TARegionState<LocationT> &s1, const TARegionState<LocationT> &s2)
{
	return std::tie(s1.location, s1.clock, s1.region_index)
	       < std::tie(s2.location, s2.clock, s2.region_index);
}

/** Check two TA region states for equality.
 * Two TA region states are considered equal if they have the same location, clock name, and region
 * index.
 * @param s1 The first state
 * @param s2 The second state
 * @return true if s1 is equal to s2
 */
template <typename LocationT>
bool
operator==(const TARegionState<LocationT> &s1, const TARegionState<LocationT> &s2)
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
using ABRegionSymbol = std::variant<TARegionState<LocationT>, ATARegionState<ConstraintSymbolType>>;

/** A canonical word H(s) for a regionalized A/B configuration */
template <typename LocationT, typename ConstraintSymbolT>
using CanonicalABWord = std::vector<std::set<ABRegionSymbol<LocationT, ConstraintSymbolT>>>;

/** Print a TARegionState. */
template <typename LocationT>
std::ostream &
operator<<(std::ostream &os, const search::TARegionState<LocationT> &state)
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
           const std::tuple<search::RegionIndex,
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
  std::ostream &                                                                           os,
  const std::vector<std::tuple<search::RegionIndex,
                               ActionType,
                               search::CanonicalABWord<LocationT, ConstraintSymbolType>>> &ab_words)
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
