/***************************************************************************
 *  type_traits.h - Type trait utility functions
 *
 *  Created:   Thu 27 Jan 18:41:25 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

namespace tacos::utilities {

// Taken from https://en.cppreference.com/w/cpp/types/disjunction.
// values_equal<a, b, T>::value is true if and only if a == b.
template <auto V1, decltype(V1) V2, typename T>
struct values_equal : std::bool_constant<V1 == V2>
{
	using type = T;
};

} // namespace tacos::utilities
