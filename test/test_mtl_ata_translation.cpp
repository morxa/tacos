/***************************************************************************
 *  test_mtl_ata_translation.cpp
 *
 *  Created: Mon 29 Jun 2020 16:33:49 CEST 16:33
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include <mtl/MTLFormula.h>
#include <mtl_ata_translation/translator.h>

#include <catch2/catch.hpp>

namespace {

using logic::AtomicProposition;
using logic::MTLFormula;
using logic::TimeInterval;

TEST_CASE("ATA satisfiability of a simple MTL formula", "[translator]")
{
	const AtomicProposition<std::string> a{"a"};
	const AtomicProposition<std::string> b{"b"};
	const MTLFormula phi = (MTLFormula{a} || MTLFormula{b}).until(MTLFormula{b}, TimeInterval(2, 3));
	const auto       ata = mtl_ata_translation::translate(phi);
	REQUIRE(ata.accepts_word({{"a", 0}, {"b", 2.5}}));
	REQUIRE(!ata.accepts_word({{"a", 0}, {"b", 1.5}}));
}

} // namespace
