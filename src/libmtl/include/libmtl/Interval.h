#pragma once

namespace arithmetic {

enum class BoundType { WEAK, STRICT, INFTY };

template <typename N>
class Interval
{
public:
	Interval(N lb, N up)
	: mLower(lb), mUpper(up), mLowerBoundType(BoundType::WEAK), mUpperBoundType(BoundType::WEAK)
	{
	}

	Interval(N lb, BoundType lbType, N up, BoundType upType)
	: mLower(lb), mUpper(up), mLowerBoundType(lbType), mUpperBoundType(upType)
	{
	}

	const N &
	lower() const noexcept
	{
		return mLower;
	}

	const N &
	upper() const noexcept
	{
		return mUpper;
	}

	BoundType
	lowerBoundType() const noexcept
	{
		return mLowerBoundType;
	}

	BoundType
	upperBoundType() const noexcept
	{
		return mUpperBoundType;
	}

private:
	N         mLower          = 0;
	N         mUpper          = 0;
	BoundType mLowerBoundType = BoundType::INFTY;
	BoundType mUpperBoundType = BoundType::INFTY;
};

} // namespace arithmetic