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

#pragma once

#include "automata.h"

#include <algorithm>
#include <memory>
#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>
#include <utility>

namespace automata {
namespace ata {

template <typename LocationT>
using State = std::pair<LocationT, ClockValuation>;

template <typename LocationT>
class Formula;

/** Print a Formula to an ostream.
 * @param os The ostream to print to
 * @param formula The formula to print
 * @return A reference to the ostream
 */
template <typename LocationT>
std::ostream &operator<<(std::ostream &os, const Formula<LocationT> &formula);

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
	//clang-format on

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
	bool
	is_satisfied(const std::set<State<LocationT>> &, const ClockValuation &) const override;
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
public:
	/** Constructor.
	 * @param location The location that must be in the configuration to satisfy this formula
	 */
	explicit LocationFormula(const LocationT &location) : location_(location){};
	bool is_satisfied(const std::set<State<LocationT>> &states, const ClockValuation &v) const override;
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

	bool
	is_satisfied(const std::set<State<LocationT>> &states, const ClockValuation &v) const override;

	std::set<std::set<State<LocationT>>>
	get_minimal_models(const ClockValuation &v) const override;

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

	bool is_satisfied(const std::set<State<LocationT>> &states, const ClockValuation &v) const override;
	std::set<std::set<State<LocationT>>> get_minimal_models(const ClockValuation &v) const override;

protected:
	/** Print a DisjunctionFormula to an ostream
	 * @param os The ostream to print to
	 */
	void
	print_to_ostream(std::ostream &os) const override;

private:
	std::unique_ptr<Formula<LocationT>> disjunct1_;
	std::unique_ptr<Formula<LocationT>> disjunct2_;
};

/// A formula that sets the clock valuation to 0 for it sub-formula
template <typename LocationT>
class ResetClockFormula : public Formula<LocationT>
{
public:
	/** Constructor.
	 * @param sub_formula The sub-formula to evaluate with a reset clock
	 */
	ResetClockFormula(std::unique_ptr<Formula<LocationT>> sub_formula)
	: sub_formula_(std::move(sub_formula))
	{
	}
	bool is_satisfied(const std::set<State<LocationT>> &states, const ClockValuation &) const override;
	std::set<std::set<State<LocationT>>> get_minimal_models(const ClockValuation &) const override;

protected:
	/** Print a ResetClockFormula to an ostream
	 * @param os The ostream to print to
	 */
	void
	print_to_ostream(std::ostream &os) const override;

private:
	std::unique_ptr<Formula<LocationT>> sub_formula_;
};

extern template class Formula<std::string>;
extern template class TrueFormula<std::string>;
extern template class FalseFormula<std::string>;
extern template class LocationFormula<std::string>;
extern template class ClockConstraintFormula<std::string>;
extern template class ConjunctionFormula<std::string>;
extern template class DisjunctionFormula<std::string>;
extern template class ResetClockFormula<std::string>;

extern template class Formula<int>;
extern template class TrueFormula<int>;
extern template class FalseFormula<int>;
extern template class LocationFormula<int>;
extern template class ClockConstraintFormula<int>;
extern template class ConjunctionFormula<int>;
extern template class DisjunctionFormula<int>;
extern template class ResetClockFormula<int>;

extern template class Formula<unsigned int>;
extern template class TrueFormula<unsigned int>;
extern template class FalseFormula<unsigned int>;
extern template class LocationFormula<unsigned int>;
extern template class ClockConstraintFormula<unsigned int>;
extern template class ConjunctionFormula<unsigned int>;
extern template class DisjunctionFormula<unsigned int>;
extern template class ResetClockFormula<unsigned int>;

} // namespace ata
} // namespace automata

/** Print a State to an ostream
 * @param os The ostream to print to
 * @param state The state to print
 * @return A reference to the ostream
 */
template <typename LocationT>
std::ostream & operator<<(std::ostream &os, const automata::ata::State<LocationT> &state);

#include "ata_formula.hpp"
