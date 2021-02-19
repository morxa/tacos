#include "mtl/MTLFormula.h"

#include "mtl/MTLFormula.hpp"

namespace logic {

template struct AtomicProposition<std::string>;
template class MTLWord<std::string>;
template class MTLFormula<std::string>;

template std::ostream &operator<<(std::ostream &out, const MTLFormula<std::string> &f);

} // namespace logic
