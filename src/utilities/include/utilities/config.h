/***************************************************************************
 *  config.h - Compile time configuration for utilities
 *
 *  Created:   Mon Dec 14 16:36:11 2020 +0100
 *  Copyright  2020  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
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

namespace utilities {
/// defines how many multiples of std::epsilon are used for absolute comparison to zero
static const int absolute_epsilon_factor = 8;
/// defines how many ulps (units in the last place) are used for float comparison
static const int max_ulps = 8;
} // namespace utilities
