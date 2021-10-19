/***************************************************************************
 *  golog_test_fixture.h - Test fixtures for gocos tests
 *
 *  Created:   Tue 19 Oct 14:38:50 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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
 *  Read the full text in the LICENSE.md file.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <semantics/readylog/history.h>
#include <semantics/readylog/utilities.h>
#pragma GCC diagnostic pop

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>

class GologTestFixture
{
public:
	GologTestFixture();

	virtual ~GologTestFixture();

protected:
	void init_program(const std::string &program);

	gologpp::SemanticsFactory *           semantics;
	std::shared_ptr<gologpp::Procedure>   main_proc;
	gologpp::Instruction *                main;
	gologpp::shared_ptr<gologpp::History> history;

private:
	bool initialized = false;
};
