#pragma once

#include "assert.h"

#include <ostream>

namespace utilities {
namespace arithmetic {

enum class BoundType { WEAK, STRICT, INFTY };

/**
 * @brief Templated class for intervals with a reduced set of operations.
 *
 * @tparam N The used number type, relevant for comparisons and arithmetic operations.
 */
template <typename N>
class Interval
{
public:
	/**
	 * @brief Construct a new Interval object
	 */
	Interval() = default;

	/**
	 * @brief Construct a new Interval object from bounds with implicit weak bound types
	 *
	 * @param lb
	 * @param up
	 */
	Interval(N lb, N up)
	: lower_(lb), upper_(up), lowerBoundType_(BoundType::WEAK), upperBoundType_(BoundType::WEAK)
	{
	}

	/**
	 * @brief Construct a new Interval object from bounds and bound types
	 *
	 * @param lb
	 * @param lbType
	 * @param up
	 * @param upType
	 */
	Interval(N lb, BoundType lbType, N up, BoundType upType)
	: lower_(lb), upper_(up), lowerBoundType_(lbType), upperBoundType_(upType)
	{
	}

	/// Check if the first interval is smaller than the second.
	/** An interval I1 is smaller than an interval I2 if:
	 * * The upper bound of I1 and the lower bound of I2 are not INFTY and
	 *   * The upper bound of I1 is smaller than the lower bound of I2 or
	 *   * The upper bound of I1 is strict and equal to the lower bound of I2 or
	 *   * The lower bound of I2 is strict  and equal to the upper bound of I1
	 * @param first The first interval to compare
	 * @param second The second interval to compare
	 * @return true if first is smaller than second
	 */
	friend bool
	operator<(const Interval &first, const Interval &second)
	{
		return first.upperBoundType_ != BoundType::INFTY && second.lowerBoundType_ != BoundType::INFTY
		       && (first.upper_ < second.lower_
		           || (first.upper_ == second.lower_
		               && (first.upperBoundType_ == BoundType::STRICT
		                   || second.lowerBoundType_ == BoundType::STRICT)));
	}

	/// Check if the first interval is bigger than the second.
	/** This is the inverse of operator<.
	 * @param first The first interval to compare
	 * @param second The second interval to compare
	 * @return true if first is bigger than second
	 * @see Interval::operator<
	 */
	friend bool
	operator>(const Interval &first, const Interval &second)
	{
		return second < first;
	}

	/// Check if two intervals are identical.
	/** Two intervals are considered to be identical if they have the same bounds and bound types. The
	 * bound value is ignored if the bound type is INFTY.
	 * @param first The first interval to compare
	 * @param second The second interval to compare
	 * @return true if neither interval is bigger
	 * @see Interval::operator<
	 */
	friend bool
	operator==(const Interval &first, const Interval &second)
	{
		return first.lowerBoundType_ == second.lowerBoundType_
		       && (first.lowerBoundType_ == BoundType::INFTY || first.lower_ == second.lower_)
		       && first.upperBoundType_ == second.upperBoundType_
		       && (first.upperBoundType_ == BoundType::INFTY || first.upper_ == second.upper_);
	}

	/// Check if two intervals are different.
	/** Two intervals are different if any bound has a different value or a different type. The bound
	 * value is ignored if the bound type is INFTY.
	 * @param first The first interval to compare
	 * @param second The second interval to compare
	 * @return true if one of the intervals is bigger than the other
	 * @see Interval::operator<
	 */
	friend bool
	operator!=(const Interval &first, const Interval &second)
	{
		return !(first == second);
	}

	/// Print an interval to an ostream.
	/** @param os The ostream to print to.
	 * @param interval The interval to print.
	 * @return A reference to the ostream.
	 */
	friend std::ostream &
	operator<<(std::ostream &os, const Interval &interval)
	{
		if (interval.lowerBoundType_ == BoundType::INFTY
		    && interval.upperBoundType_ == BoundType::INFTY) {
			// Do not print anything.
			return os;
		}
		switch (interval.lowerBoundType_) {
		case BoundType::WEAK:
		case BoundType::INFTY: os << "("; break;
		case BoundType::STRICT: os << "["; break;
		}
		if (interval.lowerBoundType_ == BoundType::INFTY) {
			os << u8"∞";
		} else {
			os << interval.lower_;
		}
		os << ", ";
		if (interval.upperBoundType_ == BoundType::INFTY) {
			os << u8"∞";
		} else {
			os << interval.upper_;
		}
		switch (interval.upperBoundType_) {
		case BoundType::WEAK:
		case BoundType::INFTY: os << ")"; break;
		case BoundType::STRICT: os << "]"; break;
		}
		return os;
	}

	/**
	 * @brief Checks containment of a value within the interval
	 * @param value
	 * @return true
	 * @return false
	 */
	bool
	contains(const N &value) const
	{
		return fitsLower(value) && fitsUpper(value);
	}

	/**
	 * @brief Checks for emptiness
	 *
	 * @return true
	 * @return false
	 */
	bool
	is_empty() const
	{
		return (lower_ > upper_ && lowerBoundType_ != BoundType::INFTY
		        && upperBoundType_ != BoundType::INFTY)
		       || (lower_ == upper_
		           && ((lowerBoundType_ == BoundType::STRICT && upperBoundType_ != BoundType::INFTY)
		               || (upperBoundType_ == BoundType::STRICT
		                   && lowerBoundType_ != BoundType::INFTY)));
	}

	/**
	 * @brief Getter for lower bound
	 *
	 * @return const N&
	 */
	const N &
	lower() const noexcept
	{
		return lower_;
	}

	/**
	 * @brief Getter for upper bound
	 *
	 * @return const N&
	 */
	const N &
	upper() const noexcept
	{
		return upper_;
	}

	/**
	 * @brief Getter for lower bound type
	 *
	 * @return BoundType
	 */
	BoundType
	lowerBoundType() const noexcept
	{
		return lowerBoundType_;
	}

	/**
	 * @brief Getter for upper bound type
	 *
	 * @return BoundType
	 */
	BoundType
	upperBoundType() const noexcept
	{
		return upperBoundType_;
	}

private:
	bool
	fitsLower(const N &value) const
	{
		return lowerBoundType_ == BoundType::INFTY || value > lower_
		       || (lowerBoundType_ == BoundType::WEAK && value >= lower_);
	}

	bool
	fitsUpper(const N &value) const
	{
		return upperBoundType_ == BoundType::INFTY || value < upper_
		       || (upperBoundType_ == BoundType::WEAK && value <= upper_);
	}

	N         lower_          = 0;
	N         upper_          = 0;
	BoundType lowerBoundType_ = BoundType::INFTY;
	BoundType upperBoundType_ = BoundType::INFTY;
};

} // namespace arithmetic
} // namespace utilities
