/***************************************************************************
 *  heuristics_generator.h - Create composite heuristics
 *
 *  Created:   Tue 27 Jul 17:25:45 CEST 2021
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
 *  Read the full text in the LICENSE.md file.
 */

#include "search/heuristics.h"

template <typename NodeT>
std::unique_ptr<search::Heuristic<long, NodeT>>
generate_heuristic(long                  weight_canonical_words     = 0,
                   long                  weight_environment_actions = 0,
                   std::set<std::string> environment_actions        = {},
                   long                  weight_time_heuristic      = 1)
{
	using H = search::Heuristic<long, NodeT>;
	std::vector<std::pair<long, std::unique_ptr<H>>> heuristics;
	heuristics.emplace_back(weight_canonical_words,
	                        std::make_unique<search::NumCanonicalWordsHeuristic<long, NodeT>>());
	heuristics.emplace_back(
	  weight_environment_actions,
	  std::make_unique<search::PreferEnvironmentActionHeuristic<long, NodeT, std::string>>(
	    environment_actions));
	heuristics.emplace_back(weight_time_heuristic,
	                        std::make_unique<search::TimeHeuristic<long, NodeT>>());
	return std::make_unique<search::CompositeHeuristic<long, NodeT>>(std::move(heuristics));
}
