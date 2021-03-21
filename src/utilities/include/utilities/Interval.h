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
	/** Interval I1 is smaller than Interval I2 if
	 * 1. the lower bound of I1 is smaller than the lower bound of I2,
	 * 2. they have the same lower bound and the upper bound of I1 is smaller than the upper bound
	 * of I2.
	 * @param first The first interval to compare
	 * @param second The second interval to compare
	 * @return true if first is smaller than second
	 */
	friend bool
	operator<(const Interval &first, const Interval &second)
	{
		auto bound_is_smaller = [](const N & first_value,
		                           BoundType first_type,
		                           const N & second_value,
		                           BoundType second_type) -> bool {
			if (first_type == BoundType::INFTY || second_type == BoundType::INFTY) {
				// Bound values do not matter as at least one bound is infty.
				// For bound types: WEAK < STRICT < INFTY
				return first_type < second_type;
			}
			if (first_value != second_value) {
				return first_value < second_value;
			}
			// Make sure our assumptions about the enumeration are correct.
			static_assert(BoundType::WEAK < BoundType::STRICT && BoundType::STRICT < BoundType::INFTY);
			// We can compare by directly comparing the bound types.
			return first_type < second_type;
		};
		return
		  // The first lower bound is smaller than the second lower bound.
		  bound_is_smaller(first.lower_, first.lowerBoundType_, second.lower_, second.lowerBoundType_)
		  ||
		  // The two lower bounds are the same.
		  (!bound_is_smaller(second.lower_, second.lowerBoundType_, first.lower_, first.lowerBoundType_)
		   // The first upper bound is smaller than the second upper bound.
		   && bound_is_smaller(
		     first.upper_, first.upperBoundType_, second.upper_, second.upperBoundType_));
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
		return !(first < second) && !(second < first);
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
