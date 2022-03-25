/***************************************************************************
 *  to_string.h - Utility functions to convert to string
 *
 *  Created:   Thu 22 Apr 08:20:52 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

#include <sstream>
#include <string>

namespace tacos::utilities {

template <typename T>
std::string
to_string(const T &v)
{
	std::stringstream str;
	str << v;
	return str.str();
}

} // namespace tacos::utilities
