#pragma once

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