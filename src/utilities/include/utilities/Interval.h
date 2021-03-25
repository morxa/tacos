#pragma once

#include "assert.h"

#include <algorithm>
#include <limits>
#include <ostream>
#include <tuple>

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
	 * @param lb The value of the lower bound (ignored if lbType == INFTY)
	 * @param lbType The type of the lower bound
	 * @param ub The value of the higher bound (ignored if ubType == INFTY)
	 * @param ubType The type of the higher bound
	 */
	Interval(N lb, BoundType lbType, N ub, BoundType ubType)
	: lower_(lb), upper_(ub), lowerBoundType_(lbType), upperBoundType_(ubType)
	{
		if (lbType == BoundType::INFTY) {
			lower_ = std::numeric_limits<N>::min();
		}
		if (ubType == BoundType::INFTY) {
			upper_ = std::numeric_limits<N>::max();
		}
	}

	/// Check if the first interval is lexicographically smaller than the second.
	/** Compare two intervals lexicographically. Note that this is not a partial
	 * order of intervals in the mathematical sense, e.g., if [a, b] < [c, d],
	 * you cannot infer that b < c. Instead, it is a total order used to put
	 * Intervals into containers.
	 * @param first The first interval to compare
	 * @param second The second interval to compare
	 * @return true if first is smaller than second
	 */
	friend bool
	operator<(const Interval &first, const Interval &second)
	{
		auto to_tuple = [](const Interval &interval) {
			return std::tie(interval.lower_,
			                interval.lowerBoundType_,
			                interval.upper_,
			                interval.upperBoundType_);
		};
		return to_tuple(first) < to_tuple(second);
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
	/** Two intervals are considered to be identical if they have the same bounds and bound types.
	 * The bound value is ignored if the bound type is INFTY.
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
	/** Two intervals are different if any bound has a different value or a different type. The
	 * bound value is ignored if the bound type is INFTY.
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

	N         lower_          = std::numeric_limits<N>::min();
	N         upper_          = std::numeric_limits<N>::max();
	BoundType lowerBoundType_ = BoundType::INFTY;
	BoundType upperBoundType_ = BoundType::INFTY;
};

} // namespace arithmetic
} // namespace utilities
