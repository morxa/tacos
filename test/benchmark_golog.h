/***************************************************************************
 *  benchmark_golog.h - Golog Benchmarks
 *
 *  Created:   Wed 19 Oct 23:49:59 CEST 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 ****************************************************************************/

#include "mtl/MTLFormula.h"

#include <set>
#include <string>

namespace tacos {

std::set<std::string>
unwrap(const std::set<logic::AtomicProposition<std::set<std::string>>> &input);

}
