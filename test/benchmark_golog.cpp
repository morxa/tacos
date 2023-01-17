/***************************************************************************
 *  benchmark_golog.cpp - Main benchmark binary for Golog benchmarks
 *
 *  Created:   Wed 28 Jul 15:57:02 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "benchmark_golog.h"

#include <benchmark/benchmark.h>

namespace tacos {
std::set<std::string>
unwrap(const std::set<logic::AtomicProposition<std::set<std::string>>> &input)
{
	std::set<std::string> res;
	for (const auto &i : input) {
		for (const auto &s : i.ap_) {
			res.insert(s);
		}
	}
	return res;
}
} // namespace tacos

BENCHMARK_MAIN();
