#pragma once
#include "Interval.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace logic {

using TimePoint    = int;
using TimeInterval = ::arithmetic::Interval<TimePoint>;

class MTLFormula;

/**
 * @brief Strong typing of atomic propositions, allows simplified construction from Boolean values
 * as well.
 *
 */
struct AtomicProposition
{
	AtomicProposition() = delete;

	/**
	 * @brief Construct a new Atomic Proposition object from string
	 *
	 * @param name
	 */
	AtomicProposition(const std::string &name) : ap_(name)
	{
	}

	/**
	 * @brief Construct a new Atomic Proposition object from bool
	 *
	 * @param b
	 */
	template <typename Y, typename T = std::enable_if_t<std::is_same<Y, bool>{}>>
	AtomicProposition(Y b)
	{
		ap_ = b ? "true" : "false";
	}

	/// less-operator
	bool
	operator<(const AtomicProposition &other) const
	{
		return ap_ < other.ap_;
	}

	std::string ap_; ///< String representation of the ap
};

/// Comparison override
bool operator==(const AtomicProposition &lhs, const AtomicProposition &rhs);

/// Not-equal operator
bool
operator!=(const AtomicProposition &lhs, const AtomicProposition &rhs)
{
	return !(rhs == lhs);
}

/**
 * @brief A (timed-) word which can be validated against a given MTL-formula.
 *
 */
class MTLWord
{
public:
	friend class MTLFormula;

	/**
	 * @brief Constructor
	 */
	MTLWord(std::initializer_list<std::pair<std::vector<AtomicProposition>, TimePoint>> a) : word_(a)
	{
	}

	/**
	 * @brief Checks satisfaction at a certain position
	 */
	bool satisfies_at(const MTLFormula &phi, std::size_t i) const;

	/**
	 * @brief Checks satisfaction at pos 0
	 */
	bool satisfies(const MTLFormula &phi) const;

private:
	std::vector<std::pair<std::vector<AtomicProposition>, TimePoint>> word_;
};

// TODO handle LDUNTIL
enum class LOP { LAND, LOR, LNEG, LUNTIL, LDUNTIL, AP };

/**
 * @brief Class representing an MTL-formula with the usual operators.
 *
 */
class MTLFormula
{
public:
	friend class MTLWord;
	MTLFormula() = delete;

	/**
	 * @brief Constructor from atomic proposition
	 */
	MTLFormula(const AtomicProposition &ap);

	/**
	 * @brief Constructor from bool
	 */
	template <typename Y, typename T = std::enable_if_t<std::is_same<Y, bool>{}>>
	MTLFormula(Y b) : ap_(b), operator_(LOP::AP)
	{
		assert(is_consistent());
	}

	/// Boolean-AND operator
	MTLFormula operator&&(const MTLFormula &rhs) const;
	/// Boolean-OR operator
	MTLFormula operator||(const MTLFormula &rhs) const;
	/// Boolean negation operator
	MTLFormula operator!() const;
	/// timed-until operator (binary)
	MTLFormula until(const MTLFormula &rhs, const TimeInterval &duration = TimeInterval()) const;
	/// timed dual_until operator (binary)
	MTLFormula dual_until(const MTLFormula &rhs, const TimeInterval &duration = TimeInterval()) const;
	/// less operator
	bool operator<(const MTLFormula &rhs) const;
	/// larger operator
	bool operator>(const MTLFormula &rhs) const;
	/// less-equal operator
	bool operator<=(const MTLFormula &rhs) const;
	/// larger-equal operator
	bool operator>=(const MTLFormula &rhs) const;
	/// equal operator
	bool operator==(const MTLFormula &rhs) const;
	/// not-equal operator
	bool operator!=(const MTLFormula &rhs) const;

	/**
	 * @brief Returns normalized formula (positive normal form)
	 * @details All negations are moved to the literals
	 * @return MTLFormula
	 */
	MTLFormula to_positive_normal_form() const;

	/// function to test whether a formula consists solely of an atomic proposition
	bool
	is_AP() const
	{
		assert(is_consistent());
		return ap_.has_value() && operator_ == LOP::AP;
	}
	/// collects all used atomic propositions of the formula
	std::set<AtomicProposition> get_alphabet() const;
	/// collects all subformulas of a specific type
	std::set<MTLFormula> get_subformulas_of_type(LOP op) const;
	/// getter for operands
	std::vector<MTLFormula>
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
	AtomicProposition
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
	           const TimeInterval &              duration = TimeInterval());

	std::optional<AtomicProposition> ap_;
	LOP                              operator_;
	std::optional<TimeInterval>      duration_ = TimeInterval();
	std::vector<MTLFormula>          operands_;
};

/// Logical AND
MTLFormula operator&&(const AtomicProposition &lhs, const AtomicProposition &rhs);
/// Logical OR
MTLFormula operator||(const AtomicProposition &lhs, const AtomicProposition &rhs);
/// Logical NEGATION
MTLFormula operator!(const AtomicProposition &ap);

} // namespace logic
