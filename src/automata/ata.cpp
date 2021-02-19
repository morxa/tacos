/***************************************************************************
 *  ata.cpp - Alternating Timed Automata
 *
 *  Created: Fri 05 Jun 2020 11:54:51 CEST 11:54
 *  Copyright  2020  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "automata/ata.h"

namespace automata::ata {

WrongTransitionTypeException::WrongTransitionTypeException(const std::string &what)
: std::logic_error(what)
{
}

NegativeTimeDeltaException::NegativeTimeDeltaException(Time time_delta)
: std::logic_error(
  "Cannot do a time transition with negative time delta (=" + std::to_string(time_delta) + ")")
{
}

template class AlternatingTimedAutomaton<std::string, std::string>;
template class AlternatingTimedAutomaton<unsigned int, std::string>;
template class AlternatingTimedAutomaton<int, std::string>;

} // namespace automata::ata

template std::ostream &operator<<(std::ostream &                                   os,
                                  const automata::ata::Configuration<std::string> &configuration);

template std::ostream &operator<<(std::ostream &                                      os,
                                  const automata::ata::Run<std::string, std::string> &run);
