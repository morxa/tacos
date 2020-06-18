#include <libmtl/MTLFormula.h>

namespace logic {

MTLFormula::MTLFormula(const AtomicProposition &ap) : ap_(ap), operator_(LOP::AP)
{
	assert(is_consistent());
}

MTLFormula::MTLFormula(LOP                               op,
                       std::initializer_list<MTLFormula> operands,
                       const TimeInterval &              duration)
: operator_(op), duration_(duration), operands_(operands)
{
	assert(is_consistent());
}

MTLFormula
MTLFormula::operator&&(const MTLFormula &rhs) const
{
	assert(is_consistent());
	return MTLFormula(LOP::LAND, {rhs});
}

MTLFormula
MTLFormula::operator||(const MTLFormula &rhs) const
{
	assert(is_consistent());
	return MTLFormula(LOP::LOR, {rhs});
}

MTLFormula MTLFormula::operator!() const
{
	assert(is_consistent());
	return MTLFormula(LOP::LNEG, {*this});
}

MTLFormula
MTLFormula::until(const MTLFormula &rhs, const TimeInterval &duration) const
{
	assert(is_consistent());
	return MTLFormula(LOP::LUNTIL, {*this, rhs}, duration);
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
		if (phi.ap_.value().ap_ == "true") {
			return true;
		}
		if (phi.ap_.value().ap_ == "false") {
			return false;
		}
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
	case LOP::LUNTIL:
		for (std::size_t j = i + 1; j < word_.size(); ++j) {
			// check if termination condition is satisfied, in time.
			if (satisfies_at(phi.operands_.back(), j)) {
				assert(phi.duration_.has_value());
				return phi.duration_.value().contains(word_[j].second - word_[i].second);
			} else {
				// check whether first part is satisfied continuously.
				if (!satisfies_at(phi.operands_.front(), j)) {
					return false;
				}
			}
		}
		// if termination condition is never met, return false.
		return false;
		break;
	default: break;
	}

	return false;
}

bool
MTLWord::satisfies(const MTLFormula &phi) const
{
	return this->satisfies_at(phi, 0);
}

} // namespace logic
