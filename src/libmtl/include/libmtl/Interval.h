#pragma once

namespace arithmetic {

enum class BoundType { WEAK, STRICT, INFTY };

template <typename N>
class Interval
{
public:
	Interval() = default;

	Interval(N lb, N up)
	: lower_(lb), upper_(up), lowerBoundType_(BoundType::WEAK), upperBoundType_(BoundType::WEAK)
	{
	}

	Interval(N lb, BoundType lbType, N up, BoundType upType)
	: lower_(lb), upper_(up), lowerBoundType_(lbType), upperBoundType_(upType)
	{
	}

	bool
	contains(const N &value) const
	{
		return fitsLower(value) && fitsUpper(value);
	}

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

	const N &
	lower() const noexcept
	{
		return lower_;
	}

	const N &
	upper() const noexcept
	{
		return upper_;
	}

	BoundType
	lowerBoundType() const noexcept
	{
		return lowerBoundType_;
	}

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