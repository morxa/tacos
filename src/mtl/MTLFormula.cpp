#include <mtl/MTLFormula.h>

namespace logic {

MTLFormula::MTLFormula(const AtomicProposition &ap) : ap_(ap), operator_(LOP::AP)
{
	assert(is_consistent());
}

MTLFormula::MTLFormula(const MTLFormula &other)
: ap_(other.ap_), operator_(other.operator_), duration_(other.duration_)
{
	std::for_each(other.get_operands().begin(), other.get_operands().end(), [&](auto &o) {
		operands_.push_back(o);
	});
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
	return MTLFormula(LOP::LAND, {*this, rhs});
}

MTLFormula
MTLFormula::operator||(const MTLFormula &rhs) const
{
	assert(is_consistent());
	return MTLFormula(LOP::LOR, {*this, rhs});
}

MTLFormula
MTLFormula::operator!() const
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

MTLFormula
MTLFormula::dual_until(const MTLFormula &rhs, const TimeInterval &duration) const
{
	assert(is_consistent());
	return MTLFormula(LOP::LDUNTIL, {*this, rhs}, duration);
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
	case LOP::LDUNTIL:
		assert(phi.duration_.has_value());
		// using  p DU q <=> !(!p U !q) (also called RELEASE operator)
		// satisfied if:
		// * q holds always, or
		// * q holds until (and including this point in time) p becomes true
		for (std::size_t j = i + 1; j < word_.size(); ++j) {
			if (satisfies_at(phi.operands_.front(), j) and satisfies_at(phi.operands_.back(), j)) {
				return phi.duration_.value().contains(word_[j].second - word_[i].second);
			} else {
				// check whether q is satisfied (probably indefinitely)
				if (!satisfies_at(phi.operands_.back(), j)) {
					return false;
				}
			}
		}
		// if q holds indefinitely, return true (1st case)
		return true;
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

bool
operator==(const AtomicProposition &lhs, const AtomicProposition &rhs)
{
	return lhs.ap_ == rhs.ap_;
}

bool
MTLFormula::operator==(const MTLFormula &rhs) const
{
	// compare operation
	if (this->get_operator() != rhs.get_operator()) {
		return false;
	}

	// base case: compare atomic propositions
	if (this->get_operator() == LOP::AP) {
		assert(rhs.get_operator() == LOP::AP);
		return this->get_atomicProposition() == rhs.get_atomicProposition();
	}

	// compare subformulas
	// Note: since the operators are the same, the size of operands needs to be the same
	if (this->get_operands().size() != rhs.get_operands().size()) {
		return false;
	}

	auto itPair = std::mismatch(this->get_operands().begin(),
	                            this->get_operands().end(),
	                            rhs.get_operands().begin());

	if (itPair.first != this->get_operands().end() && itPair.second != rhs.get_operands().end()) {
		return false;
	}

	return true;
}

bool
MTLFormula::operator!=(const MTLFormula &rhs) const
{
	return !(*this == rhs);
}

bool
MTLFormula::operator<(const MTLFormula &rhs) const
{
	// compare operation
	if (this->get_operator() != rhs.get_operator()) {
		return this->get_operator() < rhs.get_operator();
	}

	// base case: compare atomic propositions
	if (this->get_operator() == LOP::AP) {
		assert(rhs.get_operator() == LOP::AP);
		return this->get_atomicProposition() < rhs.get_atomicProposition();
	}

	// compare subformulas
	// Note: since the operators are the same, the size of operands needs to be the same
	assert(this->get_operands().size() == rhs.get_operands().size());

	return std::lexicographical_compare(this->get_operands().begin(),
	                                    this->get_operands().end(),
	                                    rhs.get_operands().begin(),
	                                    rhs.get_operands().end());
}

bool
MTLFormula::operator>(const MTLFormula &rhs) const
{
	return rhs < *this;
}

bool
MTLFormula::operator<=(const MTLFormula &rhs) const
{
	return *this < rhs || *this == rhs;
}

bool
MTLFormula::operator>=(const MTLFormula &rhs) const
{
	return *this > rhs || *this == rhs;
}

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

MTLFormula
operator!(const AtomicProposition &ap)
{
	return !MTLFormula(ap);
}

std::set<AtomicProposition>
MTLFormula::get_alphabet() const
{
	std::set<MTLFormula>               aps = get_subformulas_of_type(LOP::AP);
	std::set<logic::AtomicProposition> res;

	for (const auto &sf : aps) {
		res.insert(sf.get_atomicProposition());
	}
	return res;
}

std::set<MTLFormula>
MTLFormula::get_subformulas_of_type(LOP op) const
{
	std::set<MTLFormula> res;

	if (get_operator() == op) {
		res.insert(*this);
	}

	std::for_each(operands_.begin(), operands_.end(), [&res, op](const MTLFormula &o) {
		auto tmp = o.get_subformulas_of_type(op);
		res.insert(tmp.begin(), tmp.end());
	});

	return res;
}

std::ostream &
operator<<(std::ostream &out, const MTLFormula &f)
{
	switch (f.get_operator()) {
	case LOP::AP: out << f.get_atomicProposition(); return out;
	case LOP::LAND:
		out << "(" << f.get_operands().front() << " && " << f.get_operands().back() << ")";
		return out;
	case LOP::LOR:
		out << "(" << f.get_operands().front() << " || " << f.get_operands().back() << ")";
		return out;
	case LOP::LNEG: out << "!(" << f.get_operands().front() << ")"; return out;
	case LOP::LUNTIL:
		out << "(" << f.get_operands().front() << " U " << f.get_operands().back() << ")";
		return out;
	case LOP::LDUNTIL:
		out << "(" << f.get_operands().front() << " ~U " << f.get_operands().back() << ")";
		return out;
	default: break;
	}
	return out;
}

} // namespace logic
