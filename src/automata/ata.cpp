/***************************************************************************
 *  ata.cpp - Alternating Timed Automata
 *
 *  Created: Fri 05 Jun 2020 11:54:51 CEST 11:54
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/



#include "automata/ata.h"

namespace tacos::automata::ata {

WrongTransitionTypeException::WrongTransitionTypeException(const std::string &what)
: std::logic_error(what)
{
}

NegativeTimeDeltaException::NegativeTimeDeltaException(Time time_delta)
: std::logic_error(
  "Cannot do a time transition with negative time delta (=" + std::to_string(time_delta) + ")")
{
}

} // namespace tacos::automata::ata
