/***************************************************************************
 *  test_xml_writer.cpp - Test xml output
 *
 *  Created:   Thu 22 Jul 12:01:49 CEST 2021
 *  Copyright  2021  Stefan Schupp <stefan.schupp@tuwien.ac.at>
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

#include "io/XmlWriter.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace {

using Catch::Matchers::ContainsSubstring;

using namespace tacos;

using Location   = automata::ta::Location<std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;

TEST_CASE("Write constraint to xml", "[io]")
{
	tinyxml2::XMLDocument doc;
	auto *                root_element = doc.NewElement("root");
	doc.InsertEndChild(root_element);
	automata::ClockConstraint guard_less = automata::AtomicClockConstraintT<std::less<Time>>(1);
	automata::ClockConstraint guard_less_equal =
	  automata::AtomicClockConstraintT<std::less_equal<Time>>(2);
	automata::ClockConstraint guard_equal = automata::AtomicClockConstraintT<std::equal_to<Time>>(3);
	automata::ClockConstraint guard_greater_equal =
	  automata::AtomicClockConstraintT<std::greater_equal<Time>>(4);
	automata::ClockConstraint guard_greater = automata::AtomicClockConstraintT<std::greater<Time>>(5);

	io::add_to_uppaal_xml(std::make_pair("x", guard_less), doc, root_element);
	io::add_to_uppaal_xml(std::make_pair("x", guard_less_equal), doc, root_element);
	io::add_to_uppaal_xml(std::make_pair("x", guard_equal), doc, root_element);
	io::add_to_uppaal_xml(std::make_pair("x", guard_greater_equal), doc, root_element);
	io::add_to_uppaal_xml(std::make_pair("x", guard_greater), doc, root_element);

	tinyxml2::XMLPrinter prnt{};
	doc.SaveFile("test.xml");
	doc.Print(&prnt);
	const std::string res{prnt.CStr()};
	CHECK_THAT(res, ContainsSubstring("<root>"));
	CHECK_THAT(res, ContainsSubstring("</root>"));
	CHECK_THAT(res, ContainsSubstring("<label kind=\"guard\">x &lt; 1</label>"));
	CHECK_THAT(res, ContainsSubstring("<label kind=\"guard\">x &lt;= 2</label>"));
	CHECK_THAT(res, ContainsSubstring("<label kind=\"guard\">x == 3</label>"));
	CHECK_THAT(res, ContainsSubstring("<label kind=\"guard\">x &gt;= 4</label>"));
	CHECK_THAT(res, ContainsSubstring("<label kind=\"guard\">x &gt; 5</label>"));
}

TEST_CASE("Write transition to xml", "[io]")
{
	tinyxml2::XMLDocument doc;
	auto *                root_element = doc.NewElement("root");
	doc.InsertEndChild(root_element);
	Transition transition{Location("l0"), "a", Location("l1"), {}, {}};

	// add transition as a master-transition (channels use "!" to synchronize)
	io::add_to_uppaal_xml(transition, doc, root_element, true);

	tinyxml2::XMLPrinter prnt{};
	doc.SaveFile("test.xml");
	doc.Print(&prnt);
	const std::string res{prnt.CStr()};
	CHECK_THAT(res, ContainsSubstring("<root>"));
	CHECK_THAT(res, ContainsSubstring("</root>"));
	CHECK_THAT(res, ContainsSubstring("<transition>"));
	CHECK_THAT(res, ContainsSubstring("</transition>"));
	CHECK_THAT(res, ContainsSubstring("<source ref=\"l0\"/>"));
	CHECK_THAT(res, ContainsSubstring("<target ref=\"l1\"/>"));
	CHECK_THAT(res, ContainsSubstring("<label kind=\"synchronization\">a!</label>"));
}

TEST_CASE("Write ta to xml", "[io]")
{
	tinyxml2::XMLDocument doc;
	auto *                root_element = doc.NewElement("nta");
	doc.InsertEndChild(root_element);

	// ta, borrowed from the ta-test
	automata::ta::TimedAutomaton<std::string, std::string> ta{
	  {Location{"s0"}},
	  {"a"},
	  Location{"s0"},
	  {Location{"s0"}},
	  {"x"},
	  {Transition{Location{"s0"},
	              "a",
	              Location{"s0"},
	              {{"x", automata::AtomicClockConstraintT<std::less<Time>>(2)}},
	              {"x"}}}};

	// add transition as a master-transition (channels use "!" to synchronize)
	io::add_to_uppaal_xml(ta, doc, root_element, "simple_automaton", true);

	tinyxml2::XMLPrinter prnt{};
	doc.SaveFile("test.xml");
	doc.Print(&prnt);
	const std::string res{prnt.CStr()};
	CHECK_THAT(res, ContainsSubstring("<nta>"));
	CHECK_THAT(res, ContainsSubstring("<name>simple_automaton</name>"));
	CHECK_THAT(res, ContainsSubstring("</nta>"));
	CHECK_THAT(res, ContainsSubstring("<transition>"));
	CHECK_THAT(res, ContainsSubstring("</transition>"));
	CHECK_THAT(res, ContainsSubstring("<location id=\"s0\">"));
	CHECK_THAT(res, ContainsSubstring("<name>s0</name>"));
	CHECK_THAT(res, ContainsSubstring("</location>"));
	CHECK_THAT(res, ContainsSubstring("<source ref=\"s0\"/>"));
	CHECK_THAT(res, ContainsSubstring("<target ref=\"s0\"/>"));
	CHECK_THAT(res, ContainsSubstring("<label kind=\"guard\">x &lt; 2</label>"));
	CHECK_THAT(res, ContainsSubstring("<label kind=\"assignment\">x := 0</label>"));
	CHECK_THAT(res, ContainsSubstring("<label kind=\"synchronization\">a!</label>"));
}

TEST_CASE("Write composition to xml", "[io]")
{
	// ta, borrowed from the ta-test
	automata::ta::TimedAutomaton<std::string, std::string> master_ta{
	  {Location{"s0"}},
	  {"a"},
	  Location{"s0"},
	  {Location{"s0"}},
	  {"x"},
	  {Transition{Location{"s0"},
	              "a",
	              Location{"s0"},
	              {{"x", automata::AtomicClockConstraintT<std::less<Time>>(2)}},
	              {"x"}}}};

	std::vector<automata::ta::TimedAutomaton<std::string, std::string>> slaves{master_ta, master_ta};

	io::write_composition_to_uppaal("composition.xml", master_ta, slaves);
}

} // namespace
