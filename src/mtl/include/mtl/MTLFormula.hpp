/***************************************************************************
 *  MTLFormula.h - MTL formula representation and satisfaction
 *
 *  Created:   Thu Jun 4 13:13:54 2020 +0200
 *  Copyright  2020  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

#include "MTLFormula.h"
#include "utilities/Interval.h"
#include "utilities/numbers.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace tacos::logic {

template <typename APType>
std::ostream &
operator<<(std::ostream &out, const logic::AtomicProposition<APType> &a)
{
	if constexpr (std::is_same_v<APType, std::vector<std::string>>) {
		fmt::print(out, "({})", fmt::join(a.ap_, ", "));
		return out;
	} else {
		out << a.ap_;
		return out;
	}
}

template <typename APType>
std::ostream &
operator<<(std::ostream &out, const logic::MTLFormula<APType> &f)
{
	using logic::LOP;
	using utilities::arithmetic::BoundType;
	const auto print_until =
	  [](std::ostream &out, const logic::MTLFormula<APType> &f, std::string_view op_symbol) {
		  out << "(" << f.get_operands().front() << " " << op_symbol;
		  const auto &interval = f.get_interval();
		  // Only print interval if it is not (INFTY, INFTY), this corresponds to the usual short
		  // notation used in the literature.
		  if (interval.lowerBoundType() != BoundType::INFTY
		      || interval.upperBoundType() != BoundType::INFTY) {
			  out << interval;
		  }
		  out << " " << f.get_operands().back() << ")";
	  };
	switch (f.get_operator()) {
	case LOP::TRUE: out << u8"⊤"; break;
	case LOP::FALSE: out << u8"⊥"; break;
	case LOP::AP: out << f.get_atomicProposition(); break;
	case LOP::LAND: {
		if (f.get_operands().empty()) {
			out << u8"⊤";
			break;
		}
		if (f.get_operands().size() == 1) {
			out << f.get_operands().front();
			break;
		}
		out << "(";
		bool first = true;
		for (const auto &op : f.get_operands()) {
			if (first) {
				first = false;
			} else {
				out << " ∧ ";
			}
			out << op;
		}
		out << ")";
		break;
	}
	case LOP::LOR: {
		if (f.get_operands().empty()) {
			out << u8"⊥";
			break;
		}
		if (f.get_operands().size() == 1) {
			out << f.get_operands().front();
			break;
		}
		out << "(";
		bool first = true;
		for (const auto &op : f.get_operands()) {
			if (first) {
				first = false;
			} else {
				out << " ∨ ";
			}
			out << op;
		}
		out << ")";
		break;
	}
	case LOP::LNEG: out << "!(" << f.get_operands().front() << ")"; break;
	case LOP::LUNTIL: print_until(out, f, "U"); break;
	case LOP::LDUNTIL: print_until(out, f, "~U"); break;
	}
	return out;
}

template <typename APType>
bool
MTLWord<APType>::satisfies_at(const MTLFormula<APType> &phi, std::size_t i) const
{
	if (i >= this->word_.size())
		return false;

	switch (phi.operator_) {
	case LOP::TRUE: return true;
	case LOP::FALSE: return false;
	case LOP::AP:
		return std::find(word_[i].first.begin(), word_[i].first.end(), phi.ap_.value())
		       != word_[i].first.end();
		break;
	case LOP::LAND:
		return std::all_of(phi.operands_.begin(), phi.operands_.end(), [this, i](const auto &subf) {
			return satisfies_at(subf, i);
		});
		break;
	case LOP::LOR:
		return std::any_of(phi.operands_.begin(), phi.operands_.end(), [this, i](const auto &subf) {
			return satisfies_at(subf, i);
		});
		break;
	case LOP::LNEG:
		return std::none_of(phi.operands_.begin(), phi.operands_.end(), [this, i](const auto &subf) {
			return satisfies_at(subf, i);
		});
		break;
	case LOP::LUNTIL:
		for (std::size_t j = i + 1; j < word_.size(); ++j) {
			// check if termination condition is satisfied, in time.
			if (satisfies_at(phi.operands_.back(), j)) {
				assert(phi.duration_.has_value());
				return phi.duration_.value().contains(word_[j].second - word_[i].second);
			} else {
				// check whether first part is satisfied continuously.
				if (!satisfies_at(phi.operands_.front(), j)) {
					return false;
				}
			}
		}
		// if termination condition is never met, return false.
		return false;
		break;
	case LOP::LDUNTIL:
		assert(phi.duration_.has_value());
		// using  p DU q <=> !(!p U !q) (also called RELEASE operator)
		// satisfied if:
		// * q holds always, or
		// * q holds until (and including this point in time) p becomes true
		for (std::size_t j = i + 1; j < word_.size(); ++j) {
			if (satisfies_at(phi.operands_.front(), j) and satisfies_at(phi.operands_.back(), j)) {
				return phi.duration_.value().contains(word_[j].second - word_[i].second);
			} else {
				// check whether q is satisfied (probably indefinitely)
				if (!satisfies_at(phi.operands_.back(), j)) {
					return false;
				}
			}
		}
		// if q holds indefinitely, return true (1st case)
		return true;
		break;
	}

	return false;
}

template <typename APType>
bool
MTLWord<APType>::satisfies(const MTLFormula<APType> &phi) const
{
	return this->satisfies_at(phi, 0);
}

template <typename APType>
MTLFormula<APType>::MTLFormula(const AtomicProposition<APType> &ap) : ap_(ap), operator_(LOP::AP)
{
	assert(is_consistent());
}

template <typename APType>
MTLFormula<APType>
MTLFormula<APType>::operator&&(const MTLFormula &rhs) const
{
	assert(is_consistent());
	return MTLFormula(LOP::LAND, {*this, rhs});
}

template <typename APType>
MTLFormula<APType>
MTLFormula<APType>::operator||(const MTLFormula &rhs) const
{
	assert(is_consistent());
	return MTLFormula(LOP::LOR, {*this, rhs});
}

template <typename APType>
MTLFormula<APType>
MTLFormula<APType>::operator!() const
{
	assert(is_consistent());
	return MTLFormula(LOP::LNEG, {*this});
}

template <typename APType>
MTLFormula<APType>
MTLFormula<APType>::until(const MTLFormula &rhs, const TimeInterval &duration) const
{
	assert(is_consistent());
	return MTLFormula(LOP::LUNTIL, {*this, rhs}, duration);
}

template <typename APType>
MTLFormula<APType>
MTLFormula<APType>::dual_until(const MTLFormula &rhs, const TimeInterval &duration) const
{
	assert(is_consistent());
	return MTLFormula(LOP::LDUNTIL, {*this, rhs}, duration);
}

template <typename APType>
bool
MTLFormula<APType>::operator<(const MTLFormula &rhs) const
{
	// compare operation
	if (this->get_operator() != rhs.get_operator()) {
		return this->get_operator() < rhs.get_operator();
	}

	// base case: compare atomic propositions
	if (this->get_operator() == LOP::AP) {
		assert(rhs.get_operator() == LOP::AP);
		return this->get_atomicProposition() < rhs.get_atomicProposition();
	}

	// compare subformulas
	// Note: since the operators are the same, the size of operands needs to be the same
	assert(this->get_operands().size() == rhs.get_operands().size());

	// Compare intervals before operands.
	if (operator_ == LOP::LUNTIL || operator_ == LOP::LDUNTIL) {
		if (duration_ < rhs.duration_) {
			return true;
		}
		if (rhs.duration_ < duration_) {
			return false;
		}
	}

	return std::lexicographical_compare(this->get_operands().begin(),
	                                    this->get_operands().end(),
	                                    rhs.get_operands().begin(),
	                                    rhs.get_operands().end());
}
template <typename APType>
MTLFormula<APType>
MTLFormula<APType>::to_positive_normal_form() const
{
	switch (operator_) {
	case LOP::TRUE:
	case LOP::FALSE:
	case LOP::AP: return *this; break;
	case LOP::LNEG: {
		switch (operands_.front().get_operator()) {
		case LOP::TRUE:
		case LOP::FALSE:
		case LOP::AP: return *this; break; // negation in front of ap is conformant
		case LOP::LNEG:
			return MTLFormula(operands_.front().get_operands().front())
			  .to_positive_normal_form(); // remove duplicate negations
			break;
		case LOP::LAND:
		case LOP::LOR: {
			std::vector<MTLFormula<APType>> normalized;
			for (const auto &op : operands_.front().get_operands()) {
				normalized.push_back(MTLFormula(LOP::LNEG, {op}).to_positive_normal_form());
			}
			return MTLFormula(dual(operands_.front().get_operator()),
			                  std::begin(normalized),
			                  std::end(normalized));
		} break;
		case LOP::LUNTIL:
		case LOP::LDUNTIL: {
			// binary operators: negate operands, use dual operator
			auto neglhs =
			  MTLFormula(LOP::LNEG, {operands_.front().get_operands().front()}).to_positive_normal_form();
			auto negrhs =
			  MTLFormula(LOP::LNEG, {operands_.front().get_operands().back()}).to_positive_normal_form();
			return MTLFormula(dual(operands_.front().get_operator()),
			                  {neglhs, negrhs},
			                  operands_.front().get_interval());
		} break;
		}
	} break;
	case LOP::LAND:
	case LOP::LOR:
	case LOP::LUNTIL:
	case LOP::LDUNTIL: {
		std::vector<MTLFormula<APType>> normalized;
		for (const auto &op : operands_) {
			normalized.push_back(op.to_positive_normal_form());
		}
		auto res      = MTLFormula(operator_, std::begin(normalized), std::end(normalized));
		res.duration_ = duration_;
		return res;
	} break;
	}
	throw std::logic_error("Error in to_positive_normal_form: should have returned.");
}

template <typename APType>
std::set<AtomicProposition<APType>>
MTLFormula<APType>::get_alphabet() const
{
	std::set<MTLFormula>                       aps = get_subformulas_of_type(LOP::AP);
	std::set<logic::AtomicProposition<APType>> res;

	for (const auto &sf : aps) {
		res.insert(sf.get_atomicProposition());
	}
	return res;
}

template <typename APType>
std::set<MTLFormula<APType>>
MTLFormula<APType>::get_subformulas_of_type(LOP op) const
{
	std::set<MTLFormula> res;

	if (get_operator() == op) {
		res.insert(*this);
	}

	std::for_each(operands_.begin(), operands_.end(), [&res, op](const MTLFormula &o) {
		auto tmp = o.get_subformulas_of_type(op);
		res.insert(tmp.begin(), tmp.end());
	});

	return res;
}

template <typename APType>
TimePoint
MTLFormula<APType>::get_largest_constant() const
{
	TimePoint largest_constant = 0;
	switch (operator_) {
	case LOP::AP:
	case LOP::TRUE:
	case LOP::FALSE: largest_constant = 0; break;
	case LOP::LNEG: largest_constant = operands_[0].get_largest_constant(); break;
	case LOP::LAND:
	case LOP::LOR:
		for (const auto &sub_formula : operands_) {
			largest_constant = std::max(0., sub_formula.get_largest_constant());
		}
		break;
	case LOP::LUNTIL:
	case LOP::LDUNTIL: {
		if (duration_) {
			if (duration_->upperBoundType() != utilities::arithmetic::BoundType::INFTY) {
				largest_constant = std::max(largest_constant, duration_->upper());
			}
			if (duration_->lowerBoundType() != utilities::arithmetic::BoundType::INFTY) {
				largest_constant = std::max(largest_constant, duration_->lower());
			}
		}
		largest_constant = std::max(
		  {largest_constant, operands_[0].get_largest_constant(), operands_[1].get_largest_constant()});
		break;
	}
	}
	return largest_constant;
}

} // namespace tacos::logic
