/***************************************************************************
 *  canonical_word.h - Canonical word representation
 *
 *  Created:   Fri  5 Mar 16:38:43 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#pragma once

#include "automata/ata.h"
// TODO Regions should not be TA-specific
#include "automata/ta_regions.h"
#include "mtl/MTLFormula.h"
#include "utilities/numbers.h"
#include "utilities/types.h"

/** Get the regionalized synchronous product of a TA and an ATA. */
namespace tacos::search {

/** Always use ATA configurations over MTLFormulas */
template <typename ConstraintSymbolType>
using ATAConfiguration = automata::ata::Configuration<logic::MTLFormula<ConstraintSymbolType>>;

/** @brief The state of a plant
 *
 * An expanded state (location, clock_name, clock_valuation) of a plant.
 * A plant may be a TA or a Golog Program, depending on the template argument. */
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

/** @brief A regionalized plant state.
 *
 * A PlantRegionState is a tuple (location, clock_name, clock_region) */
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

/** @brief A regionalized ATA state.
 *
 * An ATARegionState is a pair (formula, clock_region) */
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

/** Get the clock valuation for an ABSymbol, which is either a TA state or an ATA state.
 * @param w The symbol to read the time from
 * @return The clock valuation in the given state
 */
template <typename Location, typename ConstraintSymbolType>
ClockValuation
get_time(const ABSymbol<Location, ConstraintSymbolType> &w)
{
	if (std::holds_alternative<PlantState<Location>>(w)) {
		return std::get<PlantState<Location>>(w).clock_valuation;
	} else {
		return std::get<ATAState<ConstraintSymbolType>>(w).clock_valuation;
	}
}

/** Get the clock valuation for an ABRegionSymbol, which is either a
 * TARegionState or an ATARegionState.
 * @param w The symbol to read the time from
 * @return The region index in the given state
 */
template <typename Location, typename ConstraintSymbolType>
RegionIndex
get_region_index(const ABRegionSymbol<Location, ConstraintSymbolType> &w)
{
	if (std::holds_alternative<PlantRegionState<Location>>(w)) {
		return std::get<PlantRegionState<Location>>(w).region_index;
	} else {
		return std::get<ATARegionState<ConstraintSymbolType>>(w).region_index;
	}
}

/** @brief Thrown if a canonical word is not valid. */
class InvalidCanonicalWordException : public std::domain_error
{
public:
	/** Construct the exception with a single message
	 * @param message The exact error that occurred
	 */
	explicit InvalidCanonicalWordException(const std::string &message)
	: std::domain_error(""), message_(message)
	{
	}

	/** Construct the exception with the word and a message
	 * @param word The word that is invalid
	 * @param error The error that occurred
	 */
	template <typename Location, typename ConstraintSymbolType>
	explicit InvalidCanonicalWordException(
	  const CanonicalABWord<Location, ConstraintSymbolType> &word,
	  const std::string                                     &error)
	: std::domain_error("")
	{
		std::stringstream msg;
		msg << "Invalid word: '" << word << "': " << error;
		message_ = msg.str();
	}

	/** Get the exact error message
	 * @return A message describing the error in detail
	 */
	const char *
	what() const noexcept override
	{
		return message_.c_str();
	}

private:
	std::string message_;
};

/** Validate a canonical word.
 * Check a word whether it is a valid canonical word. Throws an exception if this is not the case.
 * @param word The word to check
 * @param max_region The maximal region index that may occur in the canonical word
 * @return true if the word is a valid canonical word
 */
template <typename Location, typename ConstraintSymbolType>
bool
is_valid_canonical_word(const CanonicalABWord<Location, ConstraintSymbolType> &word,
                        RegionIndex                                            max_region = 0)
{
	// TODO all ta_symbols should agree on the same location
	// TODO clocks must have unique values (i.e., must not occur multiple times)
	if (word.empty()) {
		throw InvalidCanonicalWordException(word, "word is empty");
	}
	// No configuration should be empty
	if (std::any_of(word.begin(), word.end(), [](const auto &configurations) {
		    return configurations.empty();
	    })) {
		throw InvalidCanonicalWordException(word, "word contains an empty configuration");
	}
	// Each partition either contains only even or only odd region indexes. This is because the word
	// is partitioned by the fractional part and the region index can only be even if the fractional
	// part is 0. If that is the case, there cannot be any configuration with an odd region index in
	// the same partition, as that configuration's fractional part would be > 0.
	std::for_each(word.begin(), word.end(), [&word](const auto &configurations) {
		if (std::any_of(configurations.begin(),
		                configurations.end(),
		                [](const auto &w) { return get_region_index(w) % 2 == 0; })
		    && std::any_of(configurations.begin(), configurations.end(), [](const auto &w) {
			       return get_region_index(w) % 2 == 1;
		       })) {
			throw InvalidCanonicalWordException(word, "both odd and even region indexes");
		}
	});
	// There must be no configuration with a region larger than the max region index.
	if (max_region > 0) {
		if (std::any_of(word.begin(), word.end(), [max_region](const auto &configurations) {
			    return std::any_of(configurations.begin(),
			                       configurations.end(),
			                       [max_region](const auto &w) {
				                       return get_region_index(w) > max_region;
			                       });
		    })) {
			throw InvalidCanonicalWordException(
			  word, "word contains configuration with a region larger than the max region");
		};
	}
	// There must be at most one partition with fractional part 0.
	// The only partition that is allowed to have fracitonal part 0 is the 0th
	// partition.
	std::for_each(std::next(word.begin()), word.end(), [&word](const auto &configurations) {
		std::for_each(configurations.begin(), configurations.end(), [&word](const auto &w) {
			if (get_region_index(w) % 2 == 0) {
				throw InvalidCanonicalWordException(word,
				                                    "fractional part 0 in wrong element of partition");
			}
		});
	});
	return true;
}

/** Get the canonical word H(s) for the given A/B configuration s, closely
 * following Bouyer et al., 2006. The TAStates of s are first expanded into
 * triples (location, clock, valuation) (one for each clock), and then merged
 * with the pairs from the ATAConfiguration. The resulting set is then
 * partitioned according to the fractional part of the clock valuations. Then,
 * each tuple is regionalized by replacing the clock valuation with the
 * respective region index.  The resulting word is a sequence of sets, each set
 * containing regionalized tuples that describe a TAState or ATAState. The
 * sequence is sorted by the fractional part of the original clock valuations.
 * @param plant_configuration The configuration of the plant A (e.g., a TA
 * configuration)
 * @param ata_configuration The configuration of the alternating timed automaton B
 * @param K The value of the largest constant any clock may be compared to
 * @return The canonical word representing the state s, as a sorted vector of
 * sets of tuples (triples from A and pairs from B).
 */
template <typename Location, typename ConstraintSymbolType>
CanonicalABWord<Location, ConstraintSymbolType>
get_canonical_word(const PlantConfiguration<Location>           &plant_configuration,
                   const ATAConfiguration<ConstraintSymbolType> &ata_configuration,
                   const unsigned int                            K)
{
	using ABSymbol       = ABSymbol<Location, ConstraintSymbolType>;
	using ABRegionSymbol = ABRegionSymbol<Location, ConstraintSymbolType>;
	// TODO Also accept a TA that does not have any clocks.
	if (plant_configuration.clock_valuations.empty()) {
		throw std::invalid_argument("TA without clocks are not supported");
	}
	std::set<ABSymbol> g;
	// Insert ATA configurations into g.
	std::copy(ata_configuration.begin(), ata_configuration.end(), std::inserter(g, g.end()));
	// Insert TA configurations into g.
	for (const auto &[clock_name, clock_value] : plant_configuration.clock_valuations) {
		g.insert(PlantState<Location>{plant_configuration.location, clock_name, clock_value});
	}
	// Sort into partitions by the fractional parts.
	std::map<ClockValuation, std::set<ABSymbol>, utilities::ApproxFloatComparator<Time>>
	  partitioned_g(utilities::ApproxFloatComparator<Time>{});
	for (const ABSymbol &symbol : g) {
		partitioned_g[utilities::getFractionalPart<int, ClockValuation>(get_time(symbol))].insert(
		  symbol);
	}
	// Replace exact clock values by region indices.
	automata::ta::TimedAutomatonRegions             regionSet{K};
	CanonicalABWord<Location, ConstraintSymbolType> abs;
	for (const auto &[fractional_part, g_i] : partitioned_g) {
		std::set<ABRegionSymbol> abs_i;
		std::transform(
		  g_i.begin(),
		  g_i.end(),
		  std::inserter(abs_i, abs_i.end()),
		  [&](const ABSymbol &w) -> ABRegionSymbol {
			  if (std::holds_alternative<PlantState<Location>>(w)) {
				  const PlantState<Location> &s = std::get<PlantState<Location>>(w);
				  return PlantRegionState<Location>{s.location,
				                                    s.clock,
				                                    regionSet.getRegionIndex(s.clock_valuation)};
			  } else {
				  const ATAState<ConstraintSymbolType> &s = std::get<ATAState<ConstraintSymbolType>>(w);
				  return ATARegionState<ConstraintSymbolType>{s.location,
				                                              regionSet.getRegionIndex(s.clock_valuation)};
			  }
		  });
		abs.push_back(abs_i);
	}
	assert(is_valid_canonical_word(abs, 2 * K + 1));
	return abs;
}

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
operator<<(std::ostream                                                            &os,
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
  std::ostream                                                                         &os,
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
operator<<(std::ostream                                                                &os,
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

/** Print a multimap of (symbol, CanonicalABWord). */
template <typename ActionT, typename LocationT, typename ConstraintSymbolType>
std::ostream &
operator<<(
  std::ostream                                                                           &os,
  const std::multimap<ActionT, search::CanonicalABWord<LocationT, ConstraintSymbolType>> &ab_words)
{
	if (ab_words.empty()) {
		os << "{}";
		return os;
	}
	os << "{ ";
	bool first = true;
	for (const auto &[symbol, ab_word] : ab_words) {
		if (!first) {
			os << ", ";
		} else {
			first = false;
		}
		os << "(" << symbol << ", " << ab_word << ")";
	}
	os << " }";
	return os;
}

/** Print a next canonical word along with its region index and action. */
template <typename LocationT, typename ActionType, typename ConstraintSymbolType>
std::ostream &
operator<<(std::ostream                                                               &os,
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
operator<<(std::ostream                                                             &os,
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
