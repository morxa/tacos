/***************************************************************************
 *  golog_path_search.cpp - Utility to find paths in Golog search trees
 *  from the root node to a top or bottom labelled leaf node.
 *
 *  Created:   Tue 24 Jan 20:53:00 CET 2023
 *  Copyright  2023  Daniel Swoboda <swoboda@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "golog_adapter/golog_path_search.h"

#include "search/create_controller.h"

namespace tacos::search {
using search::get_candidate;

void
traverse_tree(tacos::search::TreeSearch<tacos::search::GologLocation,
                                        std::string,
                                        std::string,
                                        true,
                                        tacos::search::GologProgram,
                                        true>                                &search_tree,
              automata::ta::TimedAutomaton<std::set<GologWord>, GologAction> *controller)
{
	std::map<std::string, double> time_deltas{};
	traverse_node(
	  *search_tree.get_root(), tacos::search::NodeLabel::BOTTOM, controller, time_deltas, 0.0);
}

using search::get_candidate;
void
traverse_node(Node                                                           &node,
              tacos::search::NodeLabel                                        traverse_label,
              automata::ta::TimedAutomaton<std::set<GologWord>, GologAction> *controller,
              std::map<std::string, double>                                   time_deltas,
              double                                                          time)
{
	for (const auto &[timed_action, child] : node.get_children()) {
		if (child->label == traverse_label) {
			bool new_location = controller->add_location(TALocation{child->words});
			controller->add_final_location(TALocation{child->words});

			double delta = 0.0; // measure of time progression through clocks

			// get action and clock candidates for counter-example
			for (const auto &word : child->words) {
				auto candidate = get_candidate(word);
				for (auto const &clock_name : candidate.first.clock_valuations) {
					double clock_value =
					  candidate.first.clock_valuations.at(clock_name.first).get_valuation();
					if (time_deltas.find(clock_name.first) != time_deltas.end()) { // clock update
						if (clock_value > 0) {                                       // clock progression
							delta += clock_value - time_deltas[clock_name.first];
							time_deltas[clock_name.first] = clock_value;
						} else { // clock reset
							time_deltas[clock_name.first] = clock_value;
						}
					} else { // clock initialisation
						time_deltas[clock_name.first] = clock_value;
					}
				}
				break;
			}
			SPDLOG_INFO("{:10.2f} {}", time + delta, timed_action.second);

			for (const auto &[action, constraints] :
			     tacos::controller_synthesis::details::get_constraints_from_outgoing_action(node.words,
			                                                                                timed_action,
			                                                                                2)) {
				for (const auto &[clock, _constraint] : constraints) {
					controller->add_clock(clock);
				}
				controller->add_action(action);
				controller->add_transition(
				  TATransition{TALocation{node.words}, action, TALocation{child->words}, constraints, {}});
			}

			// continue to go down on the tree, if found location is new
			if (new_location) {
				traverse_node(*child, traverse_label, controller, time_deltas, time + delta);
			}
			// keep counter-example minimal
			break;
		}
	}
}

automata::ta::TimedAutomaton<std::set<GologWord>, GologAction>
verify_program(tacos::search::TreeSearch<tacos::search::GologLocation,
                                         std::string,
                                         std::string,
                                         true,
                                         tacos::search::GologProgram,
                                         true> &search_tree)
{
	if (search_tree.get_root()->label == tacos::search::NodeLabel::TOP) {
		throw std::runtime_error("Program is safe, can't create counter-example.");
	} else {
		SPDLOG_INFO("Program has unsafe execution paths, searching for a counter-example.");
		automata::ta::TimedAutomaton<std::set<GologWord>, GologAction> controller{
		  {}, TALocation{search_tree.get_root()->words}, {}};

		SPDLOG_INFO("Counter-example Trace:");
		SPDLOG_INFO("      {} {}", "time", "action");
		traverse_tree(search_tree, &controller);
		SPDLOG_INFO("---------");
		return controller;
	}
}

} // namespace tacos::search
