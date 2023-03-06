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

void
traverse_tree(tacos::search::TreeSearch<tacos::search::GologLocation,
                                        std::string,
                                        std::string,
                                        true,
                                        tacos::search::GologProgram,
                                        true>                                &search_tree,
              automata::ta::TimedAutomaton<std::set<GologWord>, GologAction> *controller)
{
	std::cout << search_tree.get_size() << std::endl;

	traverse_node(*search_tree.get_root(), 0, tacos::search::NodeLabel::BOTTOM, controller);
}

void
traverse_node(Node                                                           &node,
              int                                                             offset,
              tacos::search::NodeLabel                                        traverse_label,
              automata::ta::TimedAutomaton<std::set<GologWord>, GologAction> *controller)
{
	for (const auto &[timed_action, child] : node.get_children()) {
		if (child->label == traverse_label) {
			bool new_location = controller->add_location(TALocation{child->words});
			controller->add_final_location(TALocation{child->words});

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
				traverse_node(*child, offset + 1, traverse_label, controller);
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
		automata::ta::TimedAutomaton<std::set<GologWord>, GologAction> controller{
		  {}, TALocation{search_tree.get_root()->words}, {}};
		traverse_tree(search_tree, &controller);
		return controller;
	}
}

} // namespace tacos::search
