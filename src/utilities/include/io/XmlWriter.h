/*
 * Created by stefan on 20.07.21.
 */

#ifndef MTLSYN_XMLWRITER_H
#define MTLSYN_XMLWRITER_H

#include <automata/ta.h>
#include <fmt/format.h>

#include <sstream>
#include <tinyxml2.h>

namespace tacos::io {

/**
 * Adds an xml child-node representing a guard to the passed node.
 * @param guard The guard constraint
 * @param doc The xml document
 * @param transition_element The parent (a transition node)
 */
inline void
add_to_uppaal_xml(const std::pair<std::string, automata::ClockConstraint> &guard,
                  tinyxml2::XMLDocument &                                  doc,
                  tinyxml2::XMLElement *                                   transition_element)
{
	std::stringstream ss;
	auto *            xml_guard = doc.NewElement("label");
	xml_guard->SetAttribute("kind", "guard");
	ss << guard.first;
	if (std::holds_alternative<automata::AtomicClockConstraintT<std::less<Time>>>(guard.second)) {
		ss << " < ";
	} else if (std::holds_alternative<automata::AtomicClockConstraintT<std::less_equal<Time>>>(
	             guard.second)) {
		ss << " <= ";
	} else if (std::holds_alternative<automata::AtomicClockConstraintT<std::equal_to<Time>>>(
	             guard.second)) {
		ss << " == ";
	} else if (std::holds_alternative<automata::AtomicClockConstraintT<std::greater_equal<Time>>>(
	             guard.second)) {
		ss << " >= ";
	} else if (std::holds_alternative<automata::AtomicClockConstraintT<std::greater<Time>>>(
	             guard.second)) {
		ss << " > ";
	} else {
		throw automata::InvalidClockComparisonOperatorException();
	}
	std::visit(
	  [&ss](const auto atomic_clock_constraint) { ss << atomic_clock_constraint.get_comparand(); },
	  guard.second);
	xml_guard->SetText(ss.str().c_str());
	transition_element->InsertEndChild(xml_guard);
}

/**
 * Adds an xml child-node representing a transition to a parent node.
 * @details The flag "master" should only be true for one automaton, in our case we do not really
 * differentiate, since our semantics assumes classical synchronization without channels but with
 * labels.
 * @tparam LocationT
 * @tparam ActionT
 * @param transition The transition
 * @param doc The xml document
 * @param ta_element The parent node (a ta)
 * @param master True, if the synchronization channels are producing ("!") instead of consuming
 * ("?") labels.
 */
template <typename LocationT, typename ActionT>
void
add_to_uppaal_xml(const automata::ta::Transition<LocationT, ActionT> &transition,
                  tinyxml2::XMLDocument &                             doc,
                  tinyxml2::XMLElement *                              ta_element,
                  bool                                                master)
{
	tinyxml2::XMLElement *xml_transition = doc.NewElement("transition");
	// set source
	tinyxml2::XMLElement *source = doc.NewElement("source");
	std::stringstream     ss;
	ss << transition.get_source();
	source->SetAttribute("ref", ss.str().c_str());
	xml_transition->InsertEndChild(source);
	// target
	// clear stringstream
	ss.str(std::string());
	ss << transition.get_target();
	tinyxml2::XMLElement *target = doc.NewElement("target");
	target->SetAttribute("ref", ss.str().c_str());
	xml_transition->InsertEndChild(target);
	// set guard constraints
	for (const auto &guard_constraint : transition.get_guards()) {
		add_to_uppaal_xml(guard_constraint, doc, xml_transition);
	}
	// set clock resets
	{
		for (const auto &clock_reset : transition.get_reset()) {
			auto *xml_reset = doc.NewElement("label");
			xml_reset->SetAttribute("kind", "assignment");
			// clear stringstream
			ss.str(std::string());
			ss << clock_reset << " := 0";
			xml_reset->SetText(ss.str().c_str());
			xml_transition->InsertEndChild(xml_reset);
		}
	}

	// set synchronization labels
	{
		std::string direction = master ? "!" : "?";
		for (const auto &label : transition.get_label()) {
			// clear stringstream
			ss.str(std::string());
			auto *xml_label = doc.NewElement("label");
			xml_label->SetAttribute("kind", "synchronization");
			ss << label << direction;
			xml_label->SetText(ss.str().c_str());
			xml_transition->InsertEndChild(xml_label);
		}
	}
	// add to the ta
	ta_element->InsertEndChild(xml_transition);
}

/**
 * Adds an xml child-node representing a location to a parent node.
 * @tparam LocationT
 * @tparam ActionT
 * @param loc The location
 * @param doc The xml document
 * @param ta_element The xml parent node (a ta)
 */
template <typename LocationT, typename ActionT>
void
add_to_uppaal_xml(const automata::ta::Location<LocationT> &loc,
                  tinyxml2::XMLDocument &                  doc,
                  tinyxml2::XMLElement *                   ta_element)
{
	tinyxml2::XMLElement *xml_loc{doc.NewElement("location")};
	// store name
	std::stringstream ss;
	ss << loc;
	// set id
	xml_loc->SetAttribute("id", ss.str().c_str());
	// set name (equal to id)
	tinyxml2::XMLElement *xml_loc_name{doc.NewElement("name")};
	xml_loc_name->SetText(ss.str().c_str());
	// add to the document
	xml_loc->InsertEndChild(xml_loc_name);
	ta_element->InsertEndChild(xml_loc);
}

/**
 * Adds an xml child-node representing a ta to a parent node.
 * @tparam LocationT
 * @tparam ActionT
 * @param ta The ta
 * @param doc The xml document
 * @param root The root node of the document
 * @param name The name which should be assigned to the ta
 * @param master True, if the ta acts as a master (producing) for synchronization
 * @return A pointer to the created xml node.
 */
template <typename LocationT, typename ActionT>
tinyxml2::XMLElement *
add_to_uppaal_xml(const automata::ta::TimedAutomaton<LocationT, ActionT> &ta,
                  tinyxml2::XMLDocument &                                 doc,
                  tinyxml2::XMLElement *                                  root,
                  std::string                                             name,
                  bool                                                    master)
{
	tinyxml2::XMLElement *res = doc.NewElement("template");
	// set the name of the automaton
	tinyxml2::XMLElement *xml_name = doc.NewElement("name");
	xml_name->SetText(name.c_str());
	res->InsertEndChild(xml_name);

	// declaration: declare clocks
	tinyxml2::XMLElement *xml_declaration = doc.NewElement("declaration");
	xml_declaration->SetText(
	  fmt::format("clock {};",
	              fmt::join(std::begin(ta.get_clocks()), std::end(ta.get_clocks()), ", "))
	    .c_str());
	res->InsertEndChild(xml_declaration);

	// add locations
	for (const auto &loc : ta.get_locations()) {
		add_to_uppaal_xml<LocationT, ActionT>(loc, doc, res);
	}

	// set initial location
	tinyxml2::XMLElement *xml_initial_location = doc.NewElement("init");
	std::stringstream     ss;
	ss << ta.get_initial_location();
	xml_initial_location->SetAttribute("ref", ss.str().c_str());
	res->InsertEndChild(xml_initial_location);

	// add transitions
	for (const auto &[source, transition] : ta.get_transitions()) {
		add_to_uppaal_xml(transition, doc, res, master);
	}

	root->InsertEndChild(res);
	return res;
}

/**
 * Writes a composition of ta's to an xml file. Also adds a specification stating that it should not
 * happen that all automata are in a final location at the same time.
 * @details Uppaal-syntax is used for the xml-output. Since uppaal uses
 * master-slave/producer-consumer semantics for synchronization which is realized via channels, we
 * implement classical label synchronization via channels (one per label) and having at least one
 * (arbitrary) automaton acting as a producer and arbitrarily many automata as consumer. Since for
 * classical label synchronization this master-slave semantics is not considered, it does not matter
 * here, which of the automata is declared as "master".
 * @tparam LocationT
 * @tparam ActionT
 * @param filename The filename
 * @param master The automaton that acts as a master for synchronization
 * @param slaves The automata that act as slaves for synchronization
 */
template <typename LocationT, typename ActionT>
void
write_composition_to_uppaal(
  std::string                                                          filename,
  const automata::ta::TimedAutomaton<LocationT, ActionT> &             master,
  const std::vector<automata::ta::TimedAutomaton<LocationT, ActionT>> &slaves)
{
	tinyxml2::XMLDocument doc;
	// set root node
	tinyxml2::XMLElement *root = doc.NewElement("nta");
	doc.InsertEndChild(root);
	// add master
	tinyxml2::XMLElement *master_ptr = add_to_uppaal_xml(master, doc, root, "master", true);
	// add slaves
	std::vector<tinyxml2::XMLElement *> slave_ptrs;
	std::size_t                         count{1};
	for (const auto &slave : slaves) {
		slave_ptrs.push_back(
		  add_to_uppaal_xml(slave, doc, root, "slave" + std::to_string(count++), false));
	}
	// set up system definition
	std::vector<std::string> component_names;
	tinyxml2::XMLElement *   sys_ptr = doc.NewElement("system");
	// compose instantiation-string
	std::stringstream ss;
	ss << "M = " << master_ptr->FirstChildElement("name")->GetText() << "();\n";
	component_names.push_back("M");
	count = 1;
	for (auto *slave_ptr : slave_ptrs) {
		ss << "S" << std::to_string(count) << " = " << slave_ptr->FirstChildElement("name")->GetText()
		   << "();\n";
		component_names.push_back("S" + std::to_string(count++));
	}

	ss << fmt::format("\nsystem {};\n",
	                  fmt::join(std::begin(component_names), std::end(component_names), ", "));
	sys_ptr->SetText(ss.str().c_str());
	root->InsertEndChild(sys_ptr);

	// add specification: It should not happen, that all automata are in a final state at the same
	// time.
	std::vector<std::set<automata::ta::Location<LocationT>>> final_locations;
	// collect final locations for all components
	final_locations.push_back(master.get_final_locations());
	for (const automata::ta::TimedAutomaton<LocationT, ActionT> &slave : slaves) {
		final_locations.push_back(slave.get_final_locations());
	}
	doc.SaveFile(filename.c_str());
}

} // namespace tacos::io

#endif // MTLSYN_XMLWRITER_H
