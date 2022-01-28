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

using namespace tacos;

using MTLFormula = logic::MTLFormula<std::string>;
using AP         = logic::AtomicProposition<std::string>;

std::tuple<std::string, MTLFormula, std::set<std::string>, std::set<std::string>>
  create_crossing_problem(std::vector<tacos::Time>)
{
	std::string program = R"(
    symbol domain Location = { far, near, in, behind, far_behind }
    bool fluent train_location(Location l) {
    initially:
      (far) = true;
      (near) = false;
      (in) = false;
      (behind) = false;
      (far_behind) = false;
    }
    symbol domain GateState = { open, closed }
    symbol domain Gate = { crossing1 }
    bool fluent gate_open(Gate gate) {
    initially:
      (crossing1) = true;
    }
    action get_near() {
      precondition:
        train_location(far)
      effect:
        train_location(far) = false;
        train_location(near) = true;
    }
    action enter() {
      precondition:
        train_location(near)
      effect:
        train_location(near) = false;
        train_location(in) = true;
    }
    action leave() {
      precondition:
        train_location(in)
      effect:
        train_location(in) = false;
        train_location(behind) = true;
    }
    action travel() {
      precondition:
        train_location(behind)
      effect:
        train_location(behind) = false;
        train_location(far_behind) = true;
    }
    action close(Gate gate) {
      precondition:
        gate_open(gate)
      effect:
        gate_open(gate) = false;
    }
    action open(Gate gate) {
      precondition:
        !gate_open(gate)
      effect:
        gate_open(gate) = true;
    }

    procedure main() {
      concurrent {
        { get_near(); enter(); leave(); travel(); }
        while (!train_location(far_behind)) {
          close(crossing1); open(crossing1);
        }
      }
    }
  )";
	MTLFormula  spec    = finally(AP{"train_location(in)"} && AP{"gate_open(crossing1)"});
	return {program,
	        spec,
	        {"start_close(crossing1)", "start_open(crossing1)"},
	        {"start(get_near())",
	         "start(enter())",
	         "start(leave())",
	         "start(travel())",
	         "end_close(crossing1)",
	         "end_open(crossing1)",
	         "end(get_near())",
	         "end(enter())",
	         "end(leave())",
	         "end(travel())"}};
}
