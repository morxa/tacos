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

#include <fmt/format.h>

using namespace tacos;

using MTLFormula = logic::MTLFormula<std::string>;
using AP         = logic::AtomicProposition<std::string>;

std::tuple<std::string, MTLFormula, std::set<std::string>, std::set<std::string>>
create_robot_problem()
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
      duration: [1, 1]
      precondition:
        robot_at(from)
      effect:
        robot_at(from) = false;
        robot_at(to) = true;
    }
    action grasp(Location from, Object obj) {
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
      precondition:
        !camera_on()
      effect:
        camera_on() = true;
    }
    procedure main() {
      concurrent {
        { drive(machine1, machine2); grasp(machine2, obj1); }
        boot_camera();
      }
    }
  )";

	const MTLFormula start_drive{AP{"occ(start(drive(machine1, machine2)))"}};
	const MTLFormula end_drive{AP{"occ(end(drive(machine1, machine2)))"}};
	const MTLFormula start_boot{AP{"occ(start(boot_camera()))"}};
	const MTLFormula end_boot{AP{"occ(end(boot_camera()))"}};
	MTLFormula       spec =
	  globally((!MTLFormula{start_drive} || (!end_drive).until(end_drive, logic::TimeInterval{1, 1}))
	           && (!MTLFormula{start_boot} || (!end_boot).until(end_boot, logic::TimeInterval{1, 1})))
	  && (finally(!AP{"camera_on"} && AP{"grasping"}) || finally(MTLFormula{AP{"env_terminated"}}));
	const std::set<std::string> controller_actions  = {"ctl_terminate",
                                                    "start(drive(machine1, machine2))",
                                                    "start(grasp(machine2, obj1))",
                                                    "start(boot_camera())"};
	const std::set<std::string> environment_actions = {"env_terminate",
	                                                   "end(drive(machine1, machine2))",
	                                                   "end(grasp(machine2, obj1))",
	                                                   "end(boot_camera())"};
	return {program, spec, controller_actions, environment_actions};
}
