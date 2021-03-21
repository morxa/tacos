/***************************************************************************
 *  mtl_proto.h - Protobuf importer for MTLFormulas
 *
 *  Created:   Sat 20 Mar 21:52:29 CET 2021
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

#include "mtl/MTLFormula.h"
#include "mtl/mtl.pb.h"

namespace logic {
MTLFormula<std::string> parse_proto(const proto::MTLFormula &mtl_formula);
}
