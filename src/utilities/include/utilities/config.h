/***************************************************************************
 *  config.h - Compile time configuration for utilities
 *
 *  Created:   Mon Dec 14 16:36:11 2020 +0100
 *  Copyright  2020  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

namespace tacos::utilities {
/// defines how many multiples of std::epsilon are used for absolute comparison to zero
static const int absolute_epsilon_factor = 8;
/// defines how many ulps (units in the last place) are used for float comparison
static const int max_ulps = 8;
} // namespace tacos::utilities
