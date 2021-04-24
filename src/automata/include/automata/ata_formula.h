/***************************************************************************
 *  ata_formula.h - Alternating Timed Automata Formulas
 *
 *  Created: Thu 28 May 2020 13:39:06 CEST 13:39
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

#ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_ATA_FORMULA_H
#define SRC_AUTOMATA_INCLUDE_AUTOMATA_ATA_FORMULA_H

#include "automata.h"

#include <algorithm>
#include <memory>
#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>
#include <type_traits>
#include <utility>

namespace automata::ata {
/** A state of an ATA.
 * An ATAState is always a pair (location, clock valuation).
 * @tparam The location type
 */
template <typename LocationT>
struct State
{
	/** The location of the state */
	LocationT location;
	/** The clock valuation of the state */
	ClockValuation clock_valuation;
};

/** Compare two States.
 * @param s1 The first state
 * @param s2 The second state
 * @return true if s1 is lexicographically smaller than s2, i.e., true if s1's location is smaller
 * than s2's location, or if the locations are the same and s1's clock valuation is smaller than
 * s2's clock valuation
 */
template <typename LocationT>
bool
operator<(const State<LocationT> &s1, const State<LocationT> &s2)
{
	return std::tie(s1.location, s1.clock_valuation) < std::tie(s2.location, s2.clock_valuation);
}

/** Check two states for equality
 * @param s1 The first state
 * @param s2 The second state
 * @return true if both states are identical
 */
template <typename LocationT>
bool
operator==(const State<LocationT> &s1, const State<LocationT> &s2)
{
	return !(s1 < s2) && !(s2 < s1);
}

// using State = std::pair<LocationT, ClockValuation>;

template <typename LocationT>
class Formula;

/** Compare two ATA formulas. */
template <typename LocationT>
bool operator<(const Formula<LocationT> &, const Formula<LocationT> &);

/** Compare two ATA formulas. */
template <typename LocationT>
bool operator==(const Formula<LocationT> &, const Formula<LocationT> &);

/** Compare two ATA formulas. */
template <typename LocationT>
bool operator!=(const Formula<LocationT> &, const Formula<LocationT> &);

/** Print a Formula to an ostream.
 * @param os The ostream to print to
 * @param formula The formula to print
 * @return A reference to the ostream
 */
template <typename LocationT>
std::ostream &operator<<(std::ostream &os, const automata::ata::Formula<LocationT> &formula);

/** Print a State to an ostream
 * @param os The ostream to print to
 * @param state The state to print
 * @return A reference to the ostream
 */
template <typename LocationT>
std::ostream &operator<<(std::ostream &os, const automata::ata::State<LocationT> &state);

/// An abstract ATA formula.
template <typename LocationT>
class Formula
{
public:
	/// Constructor
	explicit Formula(){};
	Formula(const Formula &) = delete;
	virtual ~Formula()       = default;
	Formula &operator=(const Formula &) = delete;
	/** Check if the formula is satisfied by a configuration and a clock valuation.
	 * @param states The configuration to check
	 * @param v The clock valuation to check
	 * @return true if the formula is satisfied
	 */
	[[nodiscard]] virtual bool is_satisfied(const std::set<State<LocationT>> &states,
	                                        const ClockValuation &            v) const = 0;
	/** Compute the minimal model of the formula.
	 * @param v The clock valuation to evaluate teh formula against
	 * @return a set of minimal models, where each minimal model consists of a set of states
	 */
	virtual std::set<std::set<State<LocationT>>>
	get_minimal_models(const ClockValuation &v) const = 0;

	// clang-format off
	friend std::ostream & operator<< <>(std::ostream &os, const Formula &formula);
	// clang-format on

protected:
	/** A virtual method to print a Formula to an ostream. We cannot just use
	 * operator<<, as the operator cannot be virtual. The function needs to be
	 * virtual because we need to call the sub-class's implementation even when
	 * called on the base class Formula.
	 * @param os The ostream to print to
	 */
	virtual void print_to_ostream(std::ostream &os) const = 0;
};

/// A formula that is always true
template <typename LocationT>
class TrueFormula : public Formula<LocationT>
{
public:
	bool is_satisfied(const std::set<State<LocationT>> &, const ClockValuation &) const override;
	std::set<std::set<State<LocationT>>> get_minimal_models(const ClockValuation &) const override;

protected:
	/** Print a TrueFormula to an ostream
	 * @param os The ostream to print to
	 */
	void print_to_ostream(std::ostream &os) const override;
};

/// A formula that is always false
template <typename LocationT>
class FalseFormula : public Formula<LocationT>
{
public:
	bool is_satisfied(const std::set<State<LocationT>> &, const ClockValuation &) const override;
	std::set<std::set<State<LocationT>>> get_minimal_models(const ClockValuation &) const override;

protected:
	/** Print a FalseFormula to an ostream
	 * @param os The ostream to print to
	 */
	void print_to_ostream(std::ostream &os) const override;
};

/// A formula requiring a specific location
template <typename LocationT>
class LocationFormula : public Formula<LocationT>
{
	// clang-format off
	friend bool operator< <>(const Formula<LocationT> &, const Formula<LocationT> &);
	// clang-format on

public:
	/** Constructor.
	 * @param location The location that must be in the configuration to satisfy this formula
	 */
	explicit LocationFormula(const LocationT &location) : location_(location){};
	bool                                 is_satisfied(const std::set<State<LocationT>> &states,
	                                                  const ClockValuation &            v) const override;
	std::set<std::set<State<LocationT>>> get_minimal_models(const ClockValuation &v) const override;

protected:
	/** Print a LocationFormula to an ostream
	 * @param os The ostream to print to
	 */
	void print_to_ostream(std::ostream &os) const override;

private:
	const LocationT location_;
};

/// A formula requiring that a clock constraint must be satisfied
template <typename LocationT>
class ClockConstraintFormula : public Formula<LocationT>
{
	// clang-format off
	friend bool operator< <>(const Formula<LocationT> &, const Formula<LocationT> &);
	// clang-format on

public:
	/** Constructor.
	 * @param constraint The clock constraint that must be satisfied to satisfy this formula
	 */
	explicit ClockConstraintFormula(const ClockConstraint &constraint) : constraint_(constraint)
	{
	}
	bool is_satisfied(const std::set<State<LocationT>> &, const ClockValuation &v) const override;
	std::set<std::set<State<LocationT>>> get_minimal_models(const ClockValuation &v) const override;

protected:
	/** Print a ClockConstraintFormula to an ostream
	 * @param os The ostream to print to
	 */
	void print_to_ostream(std::ostream &os) const override;

private:
	ClockConstraint constraint_;
};

/// A conjunction of two formulas
template <typename LocationT>
class ConjunctionFormula : public Formula<LocationT>
{
	// clang-format off
	friend bool operator< <>(const Formula<LocationT> &, const Formula<LocationT> &);
	// clang-format on

public:
	/** Constructor.
	 * @param conjunct1 The first conjunct
	 * @param conjunct2 The second conjunct
	 */
	explicit ConjunctionFormula(std::unique_ptr<Formula<LocationT>> conjunct1,
	                            std::unique_ptr<Formula<LocationT>> conjunct2)
	: conjunct1_(std::move(conjunct1)), conjunct2_(std::move(conjunct2))
	{
	}

	bool is_satisfied(const std::set<State<LocationT>> &states,
	                  const ClockValuation &            v) const override;

	std::set<std::set<State<LocationT>>> get_minimal_models(const ClockValuation &v) const override;

protected:
	/** Print a ConjunctionFormula to an ostream
	 * @param os The ostream to print to
	 */
	void print_to_ostream(std::ostream &os) const override;

private:
	std::unique_ptr<Formula<LocationT>> conjunct1_;
	std::unique_ptr<Formula<LocationT>> conjunct2_;
};

/// A disjunction of formulas
template <typename LocationT>
class DisjunctionFormula : public Formula<LocationT>
{
	// clang-format off
	friend bool operator< <>(const Formula<LocationT> &, const Formula<LocationT> &);
	// clang-format on

public:
	/** Constructor.
	 * @param disjunct1 The first disjunct
	 * @param disjunct2 The second disjunct
	 */
	explicit DisjunctionFormula(std::unique_ptr<Formula<LocationT>> disjunct1,
	                            std::unique_ptr<Formula<LocationT>> disjunct2)
	: disjunct1_(std::move(disjunct1)), disjunct2_(std::move(disjunct2))
	{
	}

	bool                                 is_satisfied(const std::set<State<LocationT>> &states,
	                                                  const ClockValuation &            v) const override;
	std::set<std::set<State<LocationT>>> get_minimal_models(const ClockValuation &v) const override;

protected:
	/** Print a DisjunctionFormula to an ostream
	 * @param os The ostream to print to
	 */
	void print_to_ostream(std::ostream &os) const override;

private:
	std::unique_ptr<Formula<LocationT>> disjunct1_;
	std::unique_ptr<Formula<LocationT>> disjunct2_;
};

/// A formula that sets the clock valuation to 0 for it sub-formula
template <typename LocationT>
class ResetClockFormula : public Formula<LocationT>
{
	// clang-format off
	friend bool operator< <>(const Formula<LocationT> &, const Formula<LocationT> &);
	// clang-format on

public:
	/** Constructor.
	 * @param sub_formula The sub-formula to evaluate with a reset clock
	 */
	ResetClockFormula(std::unique_ptr<Formula<LocationT>> sub_formula)
	: sub_formula_(std::move(sub_formula))
	{
	}
	bool                                 is_satisfied(const std::set<State<LocationT>> &states,
	                                                  const ClockValuation &) const override;
	std::set<std::set<State<LocationT>>> get_minimal_models(const ClockValuation &) const override;

protected:
	/** Print a ResetClockFormula to an ostream
	 * @param os The ostream to print to
	 */
	void print_to_ostream(std::ostream &os) const override;

private:
	std::unique_ptr<Formula<LocationT>> sub_formula_;
};

} // namespace automata::ata

#include "ata_formula.hpp"

#endif /* ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_ATA_FORMULA_H */
