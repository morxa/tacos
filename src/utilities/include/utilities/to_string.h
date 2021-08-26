/***************************************************************************
 *  to_string.h - Utility functions to convert to string
 *
 *  Created:   Thu 22 Apr 08:20:52 CEST 2021
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
