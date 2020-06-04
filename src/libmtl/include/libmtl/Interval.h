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
	N         lower_          = 0;
	N         upper_          = 0;
	BoundType lowerBoundType_ = BoundType::INFTY;
	BoundType upperBoundType_ = BoundType::INFTY;
};

} // namespace arithmetic