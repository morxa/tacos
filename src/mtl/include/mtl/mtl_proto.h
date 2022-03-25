/***************************************************************************
 *  mtl_proto.h - Protobuf importer for MTLFormulas
 *
 *  Created:   Sat 20 Mar 21:52:29 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#pragma once

#include "mtl/MTLFormula.h"
#include "mtl/mtl.pb.h"

namespace tacos::logic {

/// Parse an MTLFormula from a proto.
/** @param mtl_formula The proto representation of an MTLFormula
 * @return The parsed MTLFormula
 */
MTLFormula<std::string> parse_proto(const proto::MTLFormula &mtl_formula);

} // namespace tacos::logic
