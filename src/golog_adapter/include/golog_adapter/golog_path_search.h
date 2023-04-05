/***************************************************************************
 *  golog_path_search.h - Utility to find paths in Golog search trees
 *  from the root node to a top or bottom labelled leaf node.
 *
 *  Created:   Tue 24 Jan 20:53:00 CET 2023
 *  Copyright  2023  Daniel Swoboda <swoboda@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "golog_adapter/golog_adapter.h"
#include "search/search.h"
#include "search/search_tree.h"

#include <iostream>
#include <string>

namespace tacos::search {

using Node            = SearchTreeNode<tacos::search::GologLocation, std::string, std::string>;
using GologAction     = std::string;
using GologConstraint = std::string;
using GologWord       = search::CanonicalABWord<tacos::search::GologLocation, GologConstraint>;
using TALocation      = automata::ta::Location<std::set<GologWord>>;
using TATransition    = automata::ta::Transition<std::set<GologWord>, GologAction>;
/**
 * @brief
 *
 * @param test
 */
void traverse_tree(tacos::search::TreeSearch<tacos::search::GologLocation,
                                             std::string,
                                             std::string,
                                             true,
                                             tacos::search::GologProgram,
                                             true> &search_tree,
                   automata::ta::TimedAutomaton<
                     std::set<search::CanonicalABWord<tacos::search::GologLocation, std::string>>,
                     std::string> *controller);

/**
 * @brief
 *
 * @param node
 */
void traverse_node(Node                    &node,
                   tacos::search::NodeLabel traverse_label,
                   automata::ta::TimedAutomaton<
                     std::set<search::CanonicalABWord<tacos::search::GologLocation, std::string>>,
                     std::string>               *controller,
                   std::map<std::string, double> time_deltas,
                   double                        time);

/**
 * @brief
 *
 * @param search_tree
 */
automata::ta::TimedAutomaton<std::set<GologWord>, GologAction>
verify_program(tacos::search::TreeSearch<tacos::search::GologLocation,
                                         std::string,
                                         std::string,
                                         true,
                                         tacos::search::GologProgram,
                                         true> &search_tree);

} // namespace tacos::search
