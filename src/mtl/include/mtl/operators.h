#pragma once
#include "MTLFormula.h"

#include <cassert>

namespace logic {
MTLFormula
operator&&(const AtomicProposition &lhs, const AtomicProposition &rhs)
{
	return MTLFormula(lhs) && MTLFormula(rhs);
}

MTLFormula
operator||(const AtomicProposition &lhs, const AtomicProposition &rhs)
{
	return MTLFormula(lhs) || MTLFormula(rhs);
}

MTLFormula operator!(const AtomicProposition &ap)
{
	return !MTLFormula(ap);
}

} // namespace logic
