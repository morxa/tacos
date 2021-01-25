#pragma once
#include "utilities/Interval.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace logic {

using TimePoint    = double;
using TimeInterval = ::utilities::arithmetic::Interval<TimePoint>;

template <typename APType>
class MTLFormula;

/// Enum for logical operation-types
enum class LOP { LAND, LOR, LNEG, LUNTIL, LDUNTIL, AP };

/// Returns the dual of the passed operator
inline LOP
dual(LOP in)
{
	switch (in) {
	case LOP::LAND: return LOP::LOR;
	case LOP::LOR: return LOP::LAND;
	case LOP::LUNTIL: return LOP::LDUNTIL;
	case LOP::LDUNTIL: return LOP::LUNTIL;
	default: return in;
	}
}

/**
 * @brief Strong typing of atomic propositions, allows simplified construction from Boolean values
 * as well.
 *
 */
template <typename APType>
struct AtomicProposition
{
	AtomicProposition() = delete;
	/// Copy constructor
	AtomicProposition(const AtomicProposition &other) = default;
	/// Copy-assignment operator
	AtomicProposition &operator=(const AtomicProposition &other) = default;

	/**
	 * @brief Construct a new Atomic Proposition object from string
	 *
	 * @param name
	 */
	AtomicProposition(const APType &name) : ap_(name)
	{
	}

	/// less-operator
	bool
	operator<(const AtomicProposition &other) const
	{
		return ap_ < other.ap_;
	}

	APType ap_; ///< String representation of the ap
};
/// outstream operator
template <typename APType>
std::ostream &
operator<<(std::ostream &out, const AtomicProposition<APType> &a)
{
	out << a.ap_;
	return out;
}

/// Comparison override
template <typename APType>
bool
operator==(const AtomicProposition<APType> &lhs, const AtomicProposition<APType> &rhs)
{
	return lhs.ap_ == rhs.ap_;
}

/// Not-equal operator
template <typename APType>
bool
operator!=(const AtomicProposition<APType> &lhs, const AtomicProposition<APType> &rhs)
{
	return !(rhs == lhs);
}

/**
 * @brief A (timed-) word which can be validated against a given MTL-formula.
 *
 */
template <typename APType>
class MTLWord
{
public:
	friend class MTLFormula<APType>;

	/**
	 * @brief Constructor
	 */
	MTLWord(std::initializer_list<std::pair<std::vector<AtomicProposition<APType>>, TimePoint>> a)
	: word_(a)
	{
	}

	/**
	 * @brief Checks satisfaction at a certain position
	 */
	bool
	satisfies_at(const MTLFormula<APType> &phi, std::size_t i) const
	{
		if (i >= this->word_.size())
			return false;

		switch (phi.operator_) {
		case LOP::AP:
			if (phi.ap_.value().ap_ == "true") {
				return true;
			}
			if (phi.ap_.value().ap_ == "false") {
				return false;
			}
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
		default: break;
		}

		return false;
	}

	/**
	 * @brief Checks satisfaction at pos 0
	 */
	bool
	satisfies(const MTLFormula<APType> &phi) const
	{
		return this->satisfies_at(phi, 0);
	}

private:
	std::vector<std::pair<std::vector<AtomicProposition<APType>>, TimePoint>> word_;
};

/**
 * @brief Class representing an MTL-formula with the usual operators.
 *
 */
template <typename APType>
class MTLFormula
{
public:
	friend class MTLWord<APType>;
	MTLFormula() = delete;

	/**
	 * @brief Constructor from atomic proposition
	 */
	MTLFormula(const AtomicProposition<APType> &ap) : ap_(ap), operator_(LOP::AP)
	{
		assert(is_consistent());
	}
	/**
	 * @brief Copy-constructor
	 * @param other
	 */
	MTLFormula(const MTLFormula &other)
	: ap_(other.ap_), operator_(other.operator_), duration_(other.duration_)
	{
		std::for_each(other.get_operands().begin(), other.get_operands().end(), [&](auto &o) {
			operands_.push_back(o);
		});
	}

	/// Boolean-AND operator
	MTLFormula
	operator&&(const MTLFormula &rhs) const
	{
		assert(is_consistent());
		return MTLFormula(LOP::LAND, {*this, rhs});
	}

	/// Boolean-OR operator
	MTLFormula
	operator||(const MTLFormula &rhs) const
	{
		assert(is_consistent());
		return MTLFormula(LOP::LOR, {*this, rhs});
	}

	/// Boolean negation operator
	MTLFormula
	operator!() const
	{
		assert(is_consistent());
		return MTLFormula(LOP::LNEG, {*this});
	}

	/// timed-until operator (binary)
	MTLFormula
	until(const MTLFormula &rhs, const TimeInterval &duration = TimeInterval()) const
	{
		assert(is_consistent());
		return MTLFormula(LOP::LUNTIL, {*this, rhs}, duration);
	}

	/// timed dual_until operator (binary)
	MTLFormula
	dual_until(const MTLFormula &rhs, const TimeInterval &duration = TimeInterval()) const
	{
		assert(is_consistent());
		return MTLFormula(LOP::LDUNTIL, {*this, rhs}, duration);
	}
	/// less operator
	bool
	operator<(const MTLFormula &rhs) const
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

		return std::lexicographical_compare(this->get_operands().begin(),
		                                    this->get_operands().end(),
		                                    rhs.get_operands().begin(),
		                                    rhs.get_operands().end());
	}
	/// larger operator
	bool
	operator>(const MTLFormula &rhs) const
	{
		return rhs < *this;
	}
	/// less-equal operator
	bool
	operator<=(const MTLFormula &rhs) const
	{
		return *this < rhs || *this == rhs;
	}
	/// larger-equal operator
	bool
	operator>=(const MTLFormula &rhs) const
	{
		return *this > rhs || *this == rhs;
	}
	/// equal operator
	bool
	operator==(const MTLFormula &rhs) const
	{
		return !(*this < rhs) && !(rhs < *this);
	}
	/// not-equal operator
	bool
	operator!=(const MTLFormula &rhs) const
	{
		return !(*this == rhs);
	}

	/**
	 * @brief Returns normalized formula (positive normal form)
	 * @details All negations are moved to the literals
	 * @return MTLFormula
	 */
	MTLFormula
	to_positive_normal_form() const
	{
		if (operator_ == LOP::LNEG) {
			switch (operands_.front().get_operator()) {
			case LOP::AP: return *this; break; // negation in front of ap is conformant
			case LOP::LNEG:
				return MTLFormula(operands_.front().get_operands().front())
				  .to_positive_normal_form(); // remove duplicate negations
				break;
			case LOP::LAND:
			case LOP::LOR:
			case LOP::LUNTIL:
			case LOP::LDUNTIL: {
				// binary operators: negate operands, use dual operator
				auto neglhs = MTLFormula(LOP::LNEG, {operands_.front().get_operands().front()})
				                .to_positive_normal_form();
				auto negrhs = MTLFormula(LOP::LNEG, {operands_.front().get_operands().back()})
				                .to_positive_normal_form();
				return MTLFormula(dual(operands_.front().get_operator()),
				                  {neglhs, negrhs},
				                  operands_.front().get_interval());
			} break;
			default: break;
			}
		}
		return *this;
	}

	/// function to test whether a formula consists solely of an atomic proposition
	bool
	is_AP() const
	{
		assert(is_consistent());
		return ap_.has_value() && operator_ == LOP::AP;
	}
	/// collects all used atomic propositions of the formula
	std::set<AtomicProposition<APType>>
	get_alphabet() const
	{
		std::set<MTLFormula>                       aps = get_subformulas_of_type(LOP::AP);
		std::set<logic::AtomicProposition<APType>> res;

		for (const auto &sf : aps) {
			res.insert(sf.get_atomicProposition());
		}
		return res;
	}
	/// collects all subformulas of a specific type
	std::set<MTLFormula<APType>>
	get_subformulas_of_type(LOP op) const
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
	/// getter for operands
	const std::vector<MTLFormula> &
	get_operands() const
	{
		return operands_;
	}
	/// getter for the logical operator
	LOP
	get_operator() const
	{
		return operator_;
	}
	/**
	 * @brief getter for the duration
	 * @details throws std::bad_optional_value exception if not set
	 * @return TimeInterval
	 */
	TimeInterval
	get_interval() const
	{
		return duration_.value();
	}

	/**
	 * @brief getter for the atomic proposition
	 * @details throws std::bad_optional_value exception if no atomic proposition was set
	 * @return AtomicProposition
	 */
	AtomicProposition<APType>
	get_atomicProposition() const
	{
		return ap_.value();
	}

private:
	bool
	is_consistent() const
	{
		return (ap_.has_value() == (operator_ == LOP::AP));
	}

	MTLFormula(LOP                               op,
	           std::initializer_list<MTLFormula> operands,
	           const TimeInterval &              duration = TimeInterval())
	: operator_(op), duration_(duration), operands_(operands)
	{
		assert(is_consistent());
	}

	std::optional<AtomicProposition<APType>> ap_;
	LOP                                      operator_;
	std::optional<TimeInterval>              duration_ = TimeInterval();
	std::vector<MTLFormula<APType>>          operands_;
};

/// Logical AND
template <typename APType>
MTLFormula<APType>
operator&&(const AtomicProposition<APType> &lhs, const AtomicProposition<APType> &rhs)
{
	return MTLFormula<APType>(lhs) && MTLFormula<APType>(rhs);
}

/// Logical OR
template <typename APType>
MTLFormula<APType>
operator||(const AtomicProposition<APType> &lhs, const AtomicProposition<APType> &rhs)
{
	return MTLFormula<APType>(lhs) || MTLFormula<APType>(rhs);
}

/// Logical NEGATION
template <typename APType>
MTLFormula<APType>
operator!(const AtomicProposition<APType> &ap)
{
	return !MTLFormula<APType>(ap);
}

/// outstream operator
template <typename APType>
std::ostream &
operator<<(std::ostream &out, const MTLFormula<APType> &f)
{
	switch (f.get_operator()) {
	case LOP::AP: out << f.get_atomicProposition(); return out;
	case LOP::LAND:
		out << "(" << f.get_operands().front() << " && " << f.get_operands().back() << ")";
		return out;
	case LOP::LOR:
		out << "(" << f.get_operands().front() << " || " << f.get_operands().back() << ")";
		return out;
	case LOP::LNEG: out << "!(" << f.get_operands().front() << ")"; return out;
	case LOP::LUNTIL:
		out << "(" << f.get_operands().front() << " U " << f.get_operands().back() << ")";
		return out;
	case LOP::LDUNTIL:
		out << "(" << f.get_operands().front() << " ~U " << f.get_operands().back() << ")";
		return out;
	default: break;
	}
	return out;
}

} // namespace logic
