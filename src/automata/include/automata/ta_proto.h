/***************************************************************************
 *  ta_proto.h - Protobuf import/export for timed automata
 *
 *  Created:   Fri 19 Mar 10:31:34 CET 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_PROTO_H
#define SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_PROTO_H

#include "automata/ta.h"
#include "automata/ta.pb.h"

namespace tacos::automata::ta {

TimedAutomaton<std::string, std::string> parse_proto(const proto::TimedAutomaton &ta_proto);
TimedAutomaton<std::vector<std::string>, std::string>
parse_product_proto(const proto::ProductAutomaton &ta_product_proto);

template <typename LocationT, typename ActionT>
proto::TimedAutomaton ta_to_proto(const TimedAutomaton<LocationT, ActionT> &ta);

namespace details {

proto::TimedAutomaton::Transition::ClockConstraint
clock_constraint_to_proto(const std::string &clock_name, const ClockConstraint &constraint);

}

} // namespace tacos::automata::ta

#include "ta_proto.hpp"

#endif /* ifndef SRC_AUTOMATA_INCLUDE_AUTOMATA_TA_PROTO_H */
