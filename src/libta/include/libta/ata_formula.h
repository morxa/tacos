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
#include <utility>

namespace automata {
namespace ata {

using State = std::pair<Location, ClockValuation>;

/// An abstract ATA formula.
class Formula
{
public:
	/// Constructor
	explicit Formula(){};
	Formula(const Formula &) = delete;
	Formula &operator=(const Formula &) = delete;
	/** Check if the formula is satisfied by a configuration and a clock valuation.
	 * @param states The configuration to check
	 * @param v The clock valuation to check
	 * @return true if the formula is satisfied
	 */
	[[nodiscard]] virtual bool is_satisfied(const std::set<State> &states,
	                                        const ClockValuation & v) const = 0;
};

/// A formula that is always true
class TrueFormula : public Formula
{
public:
	bool is_satisfied(const std::set<State> &states, const ClockValuation &v) const override;
};

/// A formula that is always false
class FalseFormula : public Formula
{
public:
	bool is_satisfied(const std::set<State> &states, const ClockValuation &v) const override;
};

/// A formula requiring a specific location
class LocationFormula : public Formula
{
public:
	/** Constructor.
	 * @param location The location that must be in the configuration to satisfy this formula
	 */
	explicit LocationFormula(const Location &location);
	bool is_satisfied(const std::set<State> &states, const ClockValuation &v) const override;

private:
	const Location location_;
};

/// A formula requiring that a clock constraint must be satisfied
class ClockConstraintFormula : public Formula
{
public:
	/** Constructor.
	 * @param constraint The clock constraint that must be satisfied to satisfy this formula
	 */
	explicit ClockConstraintFormula(const ClockConstraint &constraint);
	bool is_satisfied(const std::set<State> &states, const ClockValuation &v) const override;

private:
	ClockConstraint constraint_;
};

/// A conjunction of two formulas
class ConjunctionFormula : public Formula
{
public:
	/** Constructor.
	 * @param conjunct1 The first conjunct
	 * @param conjunct2 The second conjunct
	 */
	explicit ConjunctionFormula(std::unique_ptr<Formula> conjunct1,
	                            std::unique_ptr<Formula> conjunct2);
	bool is_satisfied(const std::set<State> &states, const ClockValuation &v) const override;

private:
	std::unique_ptr<Formula> conjunct1_;
	std::unique_ptr<Formula> conjunct2_;
};

/// A disjunction of formulas
class DisjunctionFormula : public Formula
{
public:
	/** Constructor.
	 * @param disjunct1 The first disjunct
	 * @param disjunct2 The second disjunct
	 */
	explicit DisjunctionFormula(std::unique_ptr<Formula> disjunct1,
	                            std::unique_ptr<Formula> disjunct2);
	bool is_satisfied(const std::set<State> &states, const ClockValuation &v) const override;

private:
	std::unique_ptr<Formula> disjunct1_;
	std::unique_ptr<Formula> disjunct2_;
};

/// A formula that sets the clock valuation to 0 for it sub-formula
class ResetClockFormula : public Formula
{
public:
	/** Constructor.
	 * @param sub_formula The sub-formula to evaluate with a reset clock
	 */
	ResetClockFormula(std::unique_ptr<Formula> sub_formula);
	bool is_satisfied(const std::set<State> &states, const ClockValuation &v) const override;

private:
	std::unique_ptr<Formula> sub_formula_;
};

} // namespace ata
} // namespace automata
