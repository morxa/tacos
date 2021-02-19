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

#include "automata/ata.hpp"

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

template class Transition<std::string, std::string>;
template class Transition<unsigned int, std::string>;
template class Transition<int, std::string>;

template class AlternatingTimedAutomaton<std::string, std::string>;
template class AlternatingTimedAutomaton<unsigned int, std::string>;
template class AlternatingTimedAutomaton<int, std::string>;

template bool operator<(const Transition<std::string, std::string> &first,
                        const Transition<std::string, std::string> &second);

template bool operator<(const Transition<int, std::string> &first,
                        const Transition<int, std::string> &second);

template bool operator<(const Transition<unsigned int, std::string> &first,
                        const Transition<unsigned int, std::string> &second);

template std::ostream &operator<<(std::ostream &                              os,
                                  const Transition<std::string, std::string> &transition);

template std::ostream &operator<<(std::ostream &                                             os,
                                  const AlternatingTimedAutomaton<std::string, std::string> &ata);

template std::ostream &operator<<(std::ostream &                                     os,
                                  const AlternatingTimedAutomaton<int, std::string> &ata);
template std::ostream &operator<<(std::ostream &                                              os,
                                  const AlternatingTimedAutomaton<unsigned int, std::string> &ata);

} // namespace automata::ata

template std::ostream &operator<<(std::ostream &                                   os,
                                  const automata::ata::Configuration<std::string> &configuration);

template std::ostream &operator<<(std::ostream &                                      os,
                                  const automata::ata::Run<std::string, std::string> &run);
