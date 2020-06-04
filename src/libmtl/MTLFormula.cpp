#include <libmtl/MTLFormula.h>

namespace logic {

MTLFormula::MTLFormula(const AtomicProposition &ap) : ap_(ap), operator_(LOP::AP)
{
}

MTLFormula::MTLFormula(LOP                               op,
                       std::initializer_list<MTLFormula> operands,
                       const TimeInterval &              duration)
: operator_(op), duration_(duration), operands_(operands)
{
}

MTLFormula
MTLFormula::operator&&(const MTLFormula &rhs) const
{
	return MTLFormula(LOP::LAND, {rhs});
}

MTLFormula
MTLFormula::operator||(const MTLFormula &rhs) const
{
	return MTLFormula(LOP::LOR, {rhs});
}

MTLFormula MTLFormula::operator!() const
{
	return MTLFormula(LOP::LNEG, {*this});
}

bool
operator==(const AtomicProposition &lhs, const AtomicProposition &rhs)
{
	return lhs.ap_ == rhs.ap_;
}

bool
MTLWord::satisfies_at(const MTLFormula &phi, std::size_t i) const
{
	if (i > this->word_.size())
		return false;

	switch (phi.operator_) {
	case LOP::AP:
		return std::find(word_[i].first.begin(), word_[i].first.end(), phi.ap_.value())
		       != word_[i].first.end();
		break;
	case LOP::LAND:
		return std::all_of(phi.operands_.begin(), phi.operands_.end(), [this, i](const auto &subf) {
			return satisfies_at(subf, i);
		});
		break;
	case LOP::LOR:
		return std::any_of(phi.operands_.begin(), phi.operands_.end(), [this, i](const auto &subf) {
			return satisfies_at(subf, i);
		});
		break;
	case LOP::LNEG:
		return std::none_of(phi.operands_.begin(), phi.operands_.end(), [this, i](const auto &subf) {
			return satisfies_at(subf, i);
		});
		break;
	default: break;
	}

	return false;
}

} // namespace logic
