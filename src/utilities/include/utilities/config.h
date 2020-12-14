#pragma once

namespace utilities
{
    /// defines how many multiples of std::epsilon are used for absolute comparison to zero
    static const int absolute_epsilon_factor = 4;
    /// defines how many ulps (units in the last place) are used for float comparison
    static const int max_ulps = 4;
} // namespace utilities
