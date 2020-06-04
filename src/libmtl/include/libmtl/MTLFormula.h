#pragma once
#include "Interval.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace logic {

using TimePoint    = int;
using TimeInterval = ::arithmetic::Interval<TimePoint>;

class MTLFormula;

struct AtomicProposition
{
	AtomicProposition() = delete;
	std::string ap_;
};

bool operator==(const AtomicProposition &lhs, const AtomicProposition &rhs);

class MTLWord
{
public:
	friend class MTLFormula;

	MTLWord(std::initializer_list<std::pair<std::vector<AtomicProposition>, TimePoint>> a) : word_(a)
	{
	}

	bool satisfies_at(const MTLFormula &phi, std::size_t i) const;

private:
	std::vector<std::pair<std::vector<AtomicProposition>, TimePoint>> word_;
};

enum class LOP { LAND, LOR, LNEG, AP, NOP };

class MTLFormula
{
public:
	friend class MTLWord;
	MTLFormula() = delete;
	MTLFormula(const AtomicProposition &ap);

	MTLFormula operator&&(const MTLFormula &rhs) const;
	MTLFormula operator||(const MTLFormula &rhs) const;
	MTLFormula operator!() const;

	bool
	is_AP() const
	{
		return ap_.has_value();
	}

	MTLFormula until(const MTLFormula &rhs, const TimeInterval &duration = TimeInterval()) const;

private:
	MTLFormula(LOP                               op,
	           std::initializer_list<MTLFormula> operands,
	           const TimeInterval &              duration = TimeInterval());

	std::optional<AtomicProposition> ap_;
	LOP                              operator_ = LOP::NOP;
	std::optional<TimeInterval>      duration_ = TimeInterval();
	std::vector<MTLFormula>          operands_;
};

} // namespace logic