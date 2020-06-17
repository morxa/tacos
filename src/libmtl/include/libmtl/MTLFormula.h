#pragma once
#include "Interval.h"

#include <algorithm>
#include <cassert>
#include <optional>
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

enum class LOP { LAND, LOR, LNEG, LUNTIL, AP };

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
	}

	/** Boolean AND-operator **/
	MTLFormula operator&&(const MTLFormula &rhs) const;
	/** Boolean OR-operator **/
	MTLFormula operator||(const MTLFormula &rhs) const;
	/** Boolean negation-operator **/
	MTLFormula operator!() const;
	/// timed-until operator (binary)
	MTLFormula until(const MTLFormula &rhs, const TimeInterval &duration = TimeInterval()) const;

	/// function to test whether a formula consists solely of an atomic proposition
	bool
	is_AP() const
	{
		return ap_.has_value();
	}

private:
	MTLFormula(LOP                               op,
	           std::initializer_list<MTLFormula> operands,
	           const TimeInterval &              duration = TimeInterval());

	std::optional<AtomicProposition> ap_;
	LOP                              operator_;
	std::optional<TimeInterval>      duration_ = TimeInterval();
	std::vector<MTLFormula>          operands_;
};

} // namespace logic