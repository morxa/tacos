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

using namespace tacos;

using Location   = automata::ta::Location<std::string>;
using Transition = automata::ta::Transition<std::string, std::string>;

using Catch::Matchers::Contains;

TEST_CASE("Write constraint to xml", "[io]")
{
	tinyxml2::XMLDocument doc;
	auto *                root_element = doc.NewElement("root");
	doc.InsertEndChild(root_element);
	automata::ClockConstraint guard_less =
	  automata::AtomicClockConstraintT<std::less<automata::Time>>(1);
	automata::ClockConstraint guard_less_equal =
	  automata::AtomicClockConstraintT<std::less_equal<automata::Time>>(2);
	automata::ClockConstraint guard_equal =
	  automata::AtomicClockConstraintT<std::equal_to<automata::Time>>(3);
	automata::ClockConstraint guard_greater_equal =
	  automata::AtomicClockConstraintT<std::greater_equal<automata::Time>>(4);
	automata::ClockConstraint guard_greater =
	  automata::AtomicClockConstraintT<std::greater<automata::Time>>(5);

	io::add_to_uppaal_xml(std::make_pair("x", guard_less), doc, root_element);
	io::add_to_uppaal_xml(std::make_pair("x", guard_less_equal), doc, root_element);
	io::add_to_uppaal_xml(std::make_pair("x", guard_equal), doc, root_element);
	io::add_to_uppaal_xml(std::make_pair("x", guard_greater_equal), doc, root_element);
	io::add_to_uppaal_xml(std::make_pair("x", guard_greater), doc, root_element);

	tinyxml2::XMLPrinter prnt{};
	doc.SaveFile("test.xml");
	doc.Print(&prnt);
	std::string res = std::string(prnt.CStr());
	CHECK(res.find("<root>") != std::string::npos);
	CHECK(res.find("</root>") != std::string::npos);
	CHECK(res.find("<label kind=\"guard\">x &lt; 1</label>") != std::string::npos);
	CHECK(res.find("<label kind=\"guard\">x &lt;= 2</label>") != std::string::npos);
	CHECK(res.find("<label kind=\"guard\">x == 3</label>") != std::string::npos);
	CHECK(res.find("<label kind=\"guard\">x &gt;= 4</label>") != std::string::npos);
	CHECK(res.find("<label kind=\"guard\">x &gt; 5</label>") != std::string::npos);
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
	std::string res = std::string(prnt.CStr());
	CHECK(res.find("<root>") != std::string::npos);
	CHECK(res.find("</root>") != std::string::npos);
	CHECK(res.find("<transition>") != std::string::npos);
	CHECK(res.find("</transition>") != std::string::npos);
	CHECK(res.find("<source ref=\"l0\"/>") != std::string::npos);
	CHECK(res.find("<target ref=\"l1\"/>") != std::string::npos);
	CHECK(res.find("<label kind=\"synchronization\">a!</label>") != std::string::npos);
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
	              {{"x", automata::AtomicClockConstraintT<std::less<automata::Time>>(2)}},
	              {"x"}}}};

	// add transition as a master-transition (channels use "!" to synchronize)
	io::add_to_uppaal_xml(ta, doc, root_element, "simple_automaton", true);

	tinyxml2::XMLPrinter prnt{};
	doc.SaveFile("test.xml");
	doc.Print(&prnt);
	std::string res = std::string(prnt.CStr());
	CHECK(res.find("<nta>") != std::string::npos);
	CHECK(res.find("<name>simple_automaton</name>") != std::string::npos);
	CHECK(res.find("</nta>") != std::string::npos);
	CHECK(res.find("<transition>") != std::string::npos);
	CHECK(res.find("</transition>") != std::string::npos);
	CHECK(res.find("<location id=\"s0\">") != std::string::npos);
	CHECK(res.find("<name>s0</name>") != std::string::npos);
	CHECK(res.find("</location>") != std::string::npos);
	CHECK(res.find("<source ref=\"s0\"/>") != std::string::npos);
	CHECK(res.find("<target ref=\"s0\"/>") != std::string::npos);
	CHECK(res.find("<label kind=\"guard\">x &lt; 2</label>") != std::string::npos);
	CHECK(res.find("<label kind=\"assignment\">x := 0</label>") != std::string::npos);
	CHECK(res.find("<label kind=\"synchronization\">a!</label>") != std::string::npos);
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
	              {{"x", automata::AtomicClockConstraintT<std::less<automata::Time>>(2)}},
	              {"x"}}}};

	std::vector<automata::ta::TimedAutomaton<std::string, std::string>> slaves{master_ta, master_ta};

	io::write_composition_to_uppaal("composition.xml", master_ta, slaves);
}

} // namespace