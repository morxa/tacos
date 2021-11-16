/***************************************************************************
 *  adapter.h - General plant adapter definition required for search
 *
 *  Created:   Tue 19 Oct 17:38:25 CEST 2021
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

#pragma once

#include "search/canonical_word.h"

#include <map>

namespace tacos::search {

/** Generic functor to get the next canonical word.
 * This is not actually an implementation, but only defines the interface. Instead, a plant-specific
 * (partial) template specialization needs to be defined that computes the next canonical words.
 */
template <typename Plant,
          typename ActionType,
          typename ConstraintSymbolType,
          bool use_location_constraints = false>
struct get_next_canonical_words
{
	/** Get all successors for one particular time successor. */
	std::multimap<ActionType, CanonicalABWord<typename Plant::Location, ConstraintSymbolType>>
	operator()(
	  const Plant &,
	  const automata::ata::AlternatingTimedAutomaton<logic::MTLFormula<ConstraintSymbolType>,
	                                                 logic::AtomicProposition<ConstraintSymbolType>>
	    &,
	  const std::pair<typename Plant::Configuration, ATAConfiguration<ConstraintSymbolType>> &,
	  const RegionIndex,
	  const RegionIndex)
	{
		throw std::logic_error("Missing specialization for get_next_canonical_words, did you forget to "
		                       "include the adapter specialization?");
	};
};

} // namespace tacos::search
