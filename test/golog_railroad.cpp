/***************************************************************************
 *  golog_railroad.cpp - Case study generator for Railraod scenario with Golog
 *
 *  Created:   Fri 28 Jan 11:18:01 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#include "golog_railroad.h"

#include "mtl/MTLFormula.h"

#include <fmt/format.h>

using namespace tacos;

using MTLFormula = logic::MTLFormula<std::string>;
using AP         = logic::AtomicProposition<std::string>;

std::tuple<std::string, MTLFormula, std::set<std::string>, std::set<std::string>>
create_crossing_problem(const std::vector<tacos::Time> &distances)
{
	const auto gates = [&distances]() {
		std::vector<std::string> gates;
		for (std::size_t i = 1; i <= distances.size(); i++) {
			gates.push_back(fmt::format("crossing{}", i));
			return gates;
		}
		return gates;
	}();
	const auto locations = [&distances]() {
		// if (distances.size() == 1) {
		//	return std::vector<std::string>{"far", "near", "in", "behind", "far_behind"};
		//}
		std::vector<std::string> locations = {"far"};
		for (std::size_t i = 1; i <= distances.size(); i++) {
			locations.push_back(fmt::format("near_{}", i));
			locations.push_back(fmt::format("in_{}", i));
			locations.push_back(fmt::format("behind_{}", i));
			locations.push_back(fmt::format("far_behind_{}", i));
		}
		return locations;
	}();
	const auto train_locations_init = [&locations]() {
		std::vector<std::string> init;
		init.push_back(fmt::format("({}) = true;", locations[0]));
		return init;
	}();
	const auto connected_init = [&locations]() {
		std::vector<std::string> init;
		for (std::size_t i = 0; i < locations.size() - 1; i++) {
			init.push_back(fmt::format("({}, {}) = true;", locations[i], locations[i + 1]));
		}
		return init;
	}();
	const auto [open_init, closed_init] = [&gates]() {
		std::vector<std::string> open_init;
		std::vector<std::string> closed_init;
		for (const auto &gate : gates) {
			open_init.push_back(fmt::format("({}) = true;", gate));
			closed_init.push_back(fmt::format("({}) = false;", gate));
		}
		return std::make_pair(open_init, closed_init);
	}();
	const auto main_actions = [&locations]() {
		std::vector<std::string> actions;
		for (std::size_t i = 0; i < locations.size() - 1; i++) {
			actions.push_back(fmt::format("drive({}, {})", locations[i], locations[i + 1]));
		}
		return actions;
	}();
	const auto gate_programs = [&gates]() {
		std::vector<std::string> programs;
		for (std::size_t i = 0; i < gates.size(); i++) {
			programs.push_back(
			  fmt::format(R"(
        while (!train_location({final_location})) {{
          close({crossing}); open({crossing});
        }})",
			              fmt::arg("final_location", fmt::format("far_behind_{}", gates.size())),
			              fmt::arg("crossing", gates[i])));
		}
		return programs;
	}();
	const std::string program =
	  fmt::format(R"(
    symbol domain Location = {{ {locations} }}
    bool fluent train_location(Location l) {{
    initially:
      {train_locations_init}
    }}
    bool fluent connected(Location l1, Location l2) {{
    initially:
      {connected_init}
    }}
    symbol domain Gate = {{ {gates} }}
    bool fluent gate_closed(Gate gate) {{
    initially:
      {closed_init}
    }}
    bool fluent gate_open(Gate gate) {{
    initially:
      {open_init}
    }}
    action drive(Location from, Location to) {{
      precondition:
        train_location(from) & connected(from, to)
      effect:
        train_location(from) = false;
        train_location(to) = true;
    }}
    action close(Gate gate) {{
      precondition:
        gate_open(gate)
      start_effect:
        gate_open(gate) = false;
      effect:
        gate_closed(gate) = true;
    }}
    action open(Gate gate) {{
      precondition:
        gate_closed(gate)
      start_effect:
        gate_closed(gate) = false;
      effect:
        gate_open(gate) = true;
    }}

    procedure main() {{
      concurrent {{
        {{
          {main_program};
        }}
        {gate_program}
      }}
    }}
  )",
	              fmt::arg("gates", fmt::join(gates, ", ")),
	              fmt::arg("locations", fmt::join(locations, ", ")),
	              fmt::arg("train_locations_init", fmt::join(train_locations_init, "\n      ")),
	              fmt::arg("connected_init", fmt::join(connected_init, "\n      ")),
	              fmt::arg("open_init", fmt::join(open_init, "\n      ")),
	              fmt::arg("closed_init", fmt::join(closed_init, "\n      ")),
	              fmt::arg("main_program", fmt::join(main_actions, "; ")),
	              fmt::arg("gate_program", fmt::join(gate_programs, "\n")));
	MTLFormula spec = MTLFormula{AP{"env_terminated"}} || finally(MTLFormula{AP{"env_terminated"}});
	for (std::size_t i = 1; i <= distances.size(); i++) {
		spec = spec
		       || (finally(MTLFormula{AP{fmt::format("train_location(in_{})", i)}}
		                   && !AP{fmt::format("gate_closed(crossing{})", i)}));
	}
	const auto [controller_actions, environment_actions] = [&locations, &gates]() {
		std::set<std::string> controller_actions  = {"ctl_terminate"};
		std::set<std::string> environment_actions = {"env_terminate"};
		for (std::size_t i = 0; i < locations.size() - 1; i++) {
			controller_actions.insert(
			  fmt::format("start(drive({}, {}))", locations[i], locations[i + 1]));
			environment_actions.insert(fmt::format("end(drive({}, {}))", locations[i], locations[i + 1]));
		}
		for (const auto &g : gates) {
			controller_actions.insert(fmt::format("start(close({}))", g));
			controller_actions.insert(fmt::format("start(open({}))", g));
			environment_actions.insert(fmt::format("end(close({}))", g));
			environment_actions.insert(fmt::format("end(open({}))", g));
		}
		return std::make_pair(controller_actions, environment_actions);
	}();
	return {program, spec, controller_actions, environment_actions};
}
