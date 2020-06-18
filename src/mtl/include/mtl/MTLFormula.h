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

	bool
	operator<(const AtomicProposition &other) const
	{
		return ap_ < other.ap_;
	}

	std::string ap_; ///< String representation of the ap
};

/// Comparison override
bool operator==(const AtomicProposition &lhs, const AtomicProposition &rhs);

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

	/** Boolean AND-operator **/
	MTLFormula operator&&(const MTLFormula &rhs) const;
	/** Boolean OR-operator **/
	MTLFormula operator||(const MTLFormula &rhs) const;
	/** Boolean negation-operator **/
	MTLFormula operator!() const;
	/// timed-until operator (binary)
	MTLFormula until(const MTLFormula &rhs, const TimeInterval &duration = TimeInterval()) const;
	MTLFormula dual_until(const MTLFormula &rhs, const TimeInterval &duration = TimeInterval()) const;

	bool
	operator<(const MTLFormula &) const
	{
		// TODO implement
		return true;
	}

	bool
	operator==(const MTLFormula &) const
	{
		// TODO implement
		return false;
	}

	bool
	operator==(const AtomicProposition &) const
	{
		// TODO implement
		return false;
	}

	// TODO implement
	MTLFormula to_positive_normal_form() const;

	/// function to test whether a formula consists solely of an atomic proposition
	bool
	is_AP() const
	{
		assert(is_consistent());
		return ap_.has_value() && operator_ == LOP::AP;
	}

	std::set<AtomicProposition> get_alphabet() const;

	// TODO implement
	std::set<MTLFormula> get_subformulas_of_type(LOP) const;

	std::vector<MTLFormula>
	get_operands() const
	{
		return operands_;
	}

	LOP
	get_operator() const
	{
		return operator_;
	}

	TimeInterval
	get_interval() const
	{
		// TODO implement
		return TimeInterval();
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

} // namespace logic
