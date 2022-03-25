/***************************************************************************
 *  heuristics_generator.h - Create composite heuristics
 *
 *  Created:   Tue 27 Jul 17:25:45 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include "search/heuristics.h"

template <typename NodeT>
std::unique_ptr<tacos::search::Heuristic<long, NodeT>>
generate_heuristic(long                  weight_canonical_words     = 0,
                   long                  weight_environment_actions = 0,
                   std::set<std::string> environment_actions        = {},
                   long                  weight_time_heuristic      = 1)
{
	using H = tacos::search::Heuristic<long, NodeT>;
	std::vector<std::pair<long, std::unique_ptr<H>>> heuristics;
	heuristics.emplace_back(
	  weight_canonical_words,
	  std::make_unique<tacos::search::NumCanonicalWordsHeuristic<long, NodeT>>());
	heuristics.emplace_back(
	  weight_environment_actions,
	  std::make_unique<tacos::search::PreferEnvironmentActionHeuristic<long, NodeT, std::string>>(
	    environment_actions));
	heuristics.emplace_back(weight_time_heuristic,
	                        std::make_unique<tacos::search::TimeHeuristic<long, NodeT>>());
	return std::make_unique<tacos::search::CompositeHeuristic<long, NodeT>>(std::move(heuristics));
}
