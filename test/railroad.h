/***************************************************************************
 *  railroad.h - Utility functions for railroad test scenario
 *
 *  Created: Tue 20 Apr 2021 13:00:42 CEST 13:00
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
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#pragma once

#include "automata/automata.h"
#include "automata/ta.h"
#include "mtl/MTLFormula.h"

#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

class RailroadProblem
{
public:
	RailroadProblem(std::vector<tacos::Time> distances);
	const tacos::automata::ta::TimedAutomaton<std::vector<std::string_view>, std::string_view> &
	get_plant() const
	{
		return *plant;
	}
	const tacos::logic::MTLFormula<std::string_view> &
	get_spec() const
	{
		return spec;
	};
	const std::set<std::string_view> &
	get_controller_actions() const
	{
		return controller_actions;
	};
	const std::set<std::string_view> &
	get_environment_actions() const
	{
		return environment_actions;
	};

private:
	std::set<std::string> actions;
	std::set<std::string> locations;
	std::unique_ptr<
	  tacos::automata::ta::TimedAutomaton<std::vector<std::string_view>, std::string_view>>
	                                           plant;
	tacos::logic::MTLFormula<std::string_view> spec;
	std::set<std::string_view>                 controller_actions;
	std::set<std::string_view>                 environment_actions;
};
