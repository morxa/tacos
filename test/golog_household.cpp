/***************************************************************************
 *  golog_household.cpp - Case study generator for a household robot
 *
 *  Created:   Wed 19 Oct 19:43:03 CEST 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "golog_household.h"

#include "mtl/MTLFormula.h"
#include "utilities/Interval.h"

#include <fmt/format.h>

using namespace tacos;

using MTLFormula = logic::MTLFormula<std::string>;
using AP         = logic::AtomicProposition<std::string>;

std::tuple<std::string, MTLFormula, std::set<std::string>, std::set<std::string>>
create_household_problem(RegionIndex align_time)
{
	const std::string program = R"(
    symbol domain Location = {lroom, sink, table}
    symbol domain Object = {cup1}
    bool fluent robot_at(Location l) {
      initially:
        (lroom) = true;
    }
    bool fluent moving() {
      initially:
        () = false;
    }
    bool fluent grasping() {
      initially:
        () = false;
    }
    bool fluent cup_at(Object o, Location l) {
      initially:
        (cup1, table) = true;
    }
    bool fluent aligned(Location l) {
      initially:
        (table) = false;
    }
    action move(Location from, Location to) {
      precondition:
        robot_at(from)
      start_effect:
        moving() = true;
        robot_at(from) = false;
      effect:
        moving() = false;
        robot_at(to) = true;
    }
    action grasp(Location l, Object o) {
      precondition:
        robot_at(l) & cup_at(o, l)
      start_effect:
        grasping() = true;
      effect:
        grasping() = false;
        cup_at(o, l) = false;
    }
    action align(Location l) {
      precondition:
        robot_at(l)
      effect:
        aligned(l) = true;
    }
    action back_off(Location l) {
      precondition:
        aligned(l)
      effect:
        aligned(l) = false;
    }
    procedure main() {
      concurrent {
        { move(lroom, table); grasp(table, cup1); move(table, sink); }
        if (!robot_at(sink)) { align(table); back_off(table); }
      }
    }
  )";

	const MTLFormula moving{AP{"moving()"}};
	const MTLFormula grasping{AP{"grasping()"}};
	const MTLFormula aligned{AP{"aligned(table)"}};
	using logic::TimeInterval;
	using utilities::arithmetic::BoundType;
	MTLFormula spec =
	  finally(moving && aligned) || finally(!aligned && grasping)
	  || finally(!aligned
	             && finally(grasping, TimeInterval(0, BoundType::WEAK, align_time, BoundType::WEAK)));
	const std::set<std::string> controller_actions = {
	  "start(move(lroom, table))",
	  "start(grasp(table, cup1))",
	  "start(move(table, sink))",
	  "start(align(table))",
	  "start(back_off(table))",
	};
	const std::set<std::string> environment_actions = {
	  "end(move(lroom, table))",
	  "end(grasp(table, cup1))",
	  "end(move(table, sink))",
	  "end(align(table))",
	  "end(back_off(table))",
	};
	return {program, spec, controller_actions, environment_actions};
}
