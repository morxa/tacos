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

#include "golog_robot.h"

#include "mtl/MTLFormula.h"
#include "utilities/Interval.h"

#include <fmt/format.h>

using namespace tacos;

using MTLFormula = logic::MTLFormula<std::string>;
using AP         = logic::AtomicProposition<std::string>;

std::tuple<std::string, MTLFormula, std::set<std::string>, std::set<std::string>>
create_robot_problem(unsigned int camtime)
{
	const std::string program = R"(
    symbol domain Location = { machine1, machine2 }
    symbol domain Object = { obj1 }
    bool fluent robot_at(Location l) {
    initially:
      (machine1) = true;
    }
    bool fluent obj_at(Object obj, Location l) {
    initially:
      (obj1, machine2) = true;
    }
    bool fluent holding(Object obj) {
    initially:
      (obj1) = false;
    }
    bool fluent grasping() {
      initially:
        () = false;
    }
    action drive(Location from, Location to) {
      duration: [1, 2]
      precondition:
        robot_at(from)
      effect:
        robot_at(from) = false;
        robot_at(to) = true;
    }
    action grasp(Location from, Object obj) {
      duration: [1, 1]
      precondition:
        robot_at(from) & obj_at(obj, from)
      start_effect:
        grasping() = true;
      effect:
        grasping() = false;
        obj_at(obj, from) = false;
        holding(obj) = true;
    }

    bool fluent camera_on() {
      initially:
        () = false;
    }
    action boot_camera() {
      duration: [1, 1]
      precondition:
        !camera_on()
      effect:
        camera_on() = true;
    }
    action shutdown_camera() {
      duration: [1, 1]
      precondition:
        camera_on()
      start_effect:
        camera_on() = false;
    }

    procedure main() {
      concurrent {
        { drive(machine1, machine2); grasp(machine2, obj1); }
        { boot_camera(); shutdown_camera(); }
      }
    }
  )";

	const MTLFormula camera_on{AP{"camera_on()"}};
	const MTLFormula grasping{AP{"grasping()"}};
	using logic::TimeInterval;
	using utilities::arithmetic::BoundType;
	MTLFormula spec =
	  finally(!camera_on && grasping)
	  || finally(!camera_on
	             && finally(grasping, TimeInterval(0, BoundType::WEAK, camtime, BoundType::WEAK)));
	const std::set<std::string> controller_actions  = {"start(drive(machine1, machine2))",
	                                                   "start(grasp(machine2, obj1))",
	                                                   "start(boot_camera())",
	                                                   "start(shutdown_camera())"};
	const std::set<std::string> environment_actions = {"end(drive(machine1, machine2))",
	                                                   "end(grasp(machine2, obj1))",
	                                                   "end(boot_camera())",
	                                                   "end(shutdown_camera())"};
	return {program, spec, controller_actions, environment_actions};
}
