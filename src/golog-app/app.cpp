/***************************************************************************
 *  app.cpp - The tool's main app library of the GOLOG variety
 *
 *  Created:   Tue 20 Nov 18:40:33 CEST 2022
 *  Copyright  2022  Daniel Swoboda <swoboda@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "app/app.h"

#include "automata/ta.h"
#include "automata/ta.pb.h"
#include "automata/ta_product.h"
#include "automata/ta_proto.h"
#include "automata/ta_regions.h"
#include "mtl/mtl.pb.h"
#include "mtl/mtl_proto.h"
#include "mtl_ata_translation/translator.h"
#include "search/create_controller.h"
#include "search/heuristics.h"
#include "search/search.h"
#include "search/search_tree.h"
#include "search/ta_adapter.h"
#include "visualization/interactive_tree_to_graphviz.h"
#include "visualization/ta_to_graphviz.h"
#include "visualization/tree_to_graphviz.h"

#include "utilities/Interval.h"
#include "gocos/golog_adapter.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <sys/stat.h>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace tacos::golog_app {


using search::GologProgram;
using TreeSearch = tacos::search::
  TreeSearch<search::GologLocation, std::string, std::string, true, GologProgram, true>;
namespace {
std::unique_ptr<search::Heuristic<
  long,
  TreeSearch::Node>>
create_heuristic(const std::string &name, const std::set<std::string> environment_actions = {})
{
	if (name == "time") {
		return std::make_unique<search::TimeHeuristic<long, TreeSearch::Node>>();
	} else if (name == "bfs") {
		return std::make_unique<search::BfsHeuristic<long, TreeSearch::Node>>();
	} else if (name == "dfs") {
		return std::make_unique<search::DfsHeuristic<long, TreeSearch::Node>>();
	} else if (name == "random") {
		return std::make_unique<search::RandomHeuristic<long, TreeSearch::Node>>();
	} else if (name == "composite") {
		const long weight_canonical_words     = 16;
		const long weight_environment_actions = 4;
		const long weight_time                = 1;
		std::vector<std::pair<long, std::unique_ptr<search::Heuristic<long, TreeSearch::Node>>>> heuristics;
		heuristics.emplace_back(weight_canonical_words,
		                        std::make_unique<search::NumCanonicalWordsHeuristic<long, TreeSearch::Node>>());
		heuristics.emplace_back(
		  weight_environment_actions,
		  std::make_unique<search::PreferEnvironmentActionHeuristic<long, TreeSearch::Node, std::string>>(
		    environment_actions));
		heuristics.emplace_back(weight_time, std::make_unique<search::TimeHeuristic<long, TreeSearch::Node>>());

		return std::make_unique<tacos::search::CompositeHeuristic<long, TreeSearch::Node>>(std::move(heuristics));

	} else {
		throw std::invalid_argument("Unknown heuristic: " + name);
	}
}

} // namespace

Launcher::Launcher(int argc, const char *const argv[])
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	spdlog::set_pattern("%v");
	spdlog::set_level(spdlog::level::info);
	parse_command_line(argc, argv);
}

void
Launcher::parse_command_line(int argc, const char *const argv[])
{
	using boost::program_options::bool_switch;
	using boost::program_options::value;
	boost::program_options::options_description options("Allowed options");

	// clang-format off
	options.add_options()
    ("help,h", "Print help message")
    ("program,p", value(&program_path)->required(), "The path to the golog program file")
    ("specification,s", value(&specification_path)->required(), "The path to the specification proto")
    ("k", value(&K)->required(), "The maximum constant occuring in a clock constraint")
    ("single-threaded", bool_switch()->default_value(false), "run single-threaded")
    ("debug,d", bool_switch()->default_value(false), "Debug the search graph interactively")
    ("visualize-search-tree", value(&tree_dot_graph), "Generate a dot graph of the search tree")
    ("visualize-controller", value(&controller_dot_path), "Generate a dot graph of the resulting controller")
    ("hide-controller-labels", bool_switch()->default_value(true),
     "Generate a compact controller dot graph without node labels")
    ("output,o", value(&controller_proto_path), "Save the resulting controller as pbtxt")
	("controller-action,c", value<std::vector<std::string>>(), "The actions controlled by the controller")
	("environment-action,e", value<std::vector<std::string>>(), "The actions controlled by the environment")
    ("heuristic", value(&heuristic)->default_value("time"), "The heuristic to use (one of 'time', 'bfs', 'dfs')")
	;
    boost::program_options::variables_map variables;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options),
	                              variables);

	for (const auto& it : variables) {
		std::cout << it.first.c_str() << " ";
		auto& value = it.second.value();
		if (auto v = boost::any_cast<uint32_t>(&value))
			std::cout << *v << std::endl;
		else if (auto v = boost::any_cast<std::string>(&value))
			std::cout << *v << std::endl;
		else if (auto v = boost::any_cast<std::filesystem::path>(&value))
			std::cout << *v << std::endl;
		else
			std::cout << "error" << std::endl;
	}

	if (variables.count("help")) {
		SPDLOG_INFO(options);
		show_help = true;
		return;
	}
	boost::program_options::notify(variables);
	debug                  = variables["debug"].as<bool>();
	multi_threaded         = !variables["single-threaded"].as<bool>();
	hide_controller_labels = variables["hide-controller-labels"].as<bool>();

	// Convert the vector of actions into a set of actions.
	if (variables.count("controller-action")) {
		std::copy(std::begin(variables["controller-action"].as<std::vector<std::string>>()),
		          std::end(variables["controller-action"].as<std::vector<std::string>>()),
		          std::inserter(controller_actions, std::end(controller_actions)));
	}
	// Convert the vector of actions into a set of actions.
	if (variables.count("environment-action")) {
		std::copy(std::begin(variables["environment-action"].as<std::vector<std::string>>()),
		          std::end(variables["environment-action"].as<std::vector<std::string>>()),
		          std::inserter(environment_actions, std::end(environment_actions)));
	}
}

void
read_proto_from_file(const std::filesystem::path &path, google::protobuf::Message *output)
{
	int fd = open(path.c_str(), O_RDONLY);
	if (fd < 0) {
		throw std::invalid_argument(
		  fmt::format("Could not open spec file '{}' (errno: {})", path.c_str(), errno));
	}
	google::protobuf::io::FileInputStream stream(fd);
	stream.SetCloseOnDelete(true);
	if (!google::protobuf::TextFormat::Parse(&stream, output)) {
		throw std::invalid_argument(fmt::format("Failed to read proto from file '{}", path.c_str()));
	}
}

GologProgram
read_golog_from_file(const std::filesystem::path &path, std::set<std::string> relevant_fluents)
{
	std::ifstream f(path.c_str());
	if(!f) {
		throw std::invalid_argument(
		  fmt::format("Could not open golog file '{}' (errno: {})", path.c_str(), errno));
	}
	std::ostringstream ss;
	ss << f.rdbuf();

	return GologProgram(ss.str(), relevant_fluents);
}

std::set<std::string>
unwrap_fluents(std::set<logic::AtomicProposition<std::set<std::string>>> input)
{
	std::set<std::string> res;
	for (const auto &i : input) {
		for (const auto &s : i.ap_) {
			res.insert(s);
		}
	}
	return res;
}

void
Launcher::run()
{
	if (show_help) {
		return;
	}

	SPDLOG_INFO("Using maximum clock constraint K={}", K);

	// read MTL specification
	SPDLOG_INFO("Reading MTL specification of undesired behaviors from '{}'",
	            specification_path.c_str());
	logic::proto::MTLFormula spec_proto;
	read_proto_from_file(specification_path, &spec_proto);
	auto spec = logic::parse_proto(spec_proto);
	auto ata = mtl_ata_translation::translate<std::string, std::set<std::string>, true>(spec);
	SPDLOG_DEBUG("Specification: {}", spec);
	SPDLOG_DEBUG("ATA:\n{}", ata);

	// read golog program
	SPDLOG_INFO("Reading GOLOG program from '{}'",
	            program_path.c_str());
	auto program = read_golog_from_file(program_path, unwrap_fluents(ata.get_alphabet()));

	// actions are for now passed as arguments, later they should be synthesized from code
	SPDLOG_INFO("Controller actions: {}", fmt::join(controller_actions, ", "));
	SPDLOG_INFO("Environment actions: {}", fmt::join(environment_actions, ", "));

	// search
	SPDLOG_INFO("Initializing search");
	TreeSearch search(&program,
							  &ata,
							  controller_actions,
							  environment_actions,
							  K,
							  true,
							  true,
							  create_heuristic(heuristic));
	SPDLOG_INFO("Running search {}", multi_threaded ? "multi-threaded" : "single-threaded");
	search.build_tree(false);
	search.label();
	SPDLOG_INFO("Search complete!");
	if (debug) {
		if (tree_dot_graph.empty()) {
			SPDLOG_ERROR("Debugging enabled but no output file given, please specify the path to the "
			             "desired search graph output file");
			return;
		}
		visualization::search_tree_to_graphviz_interactive(search.get_root(), tree_dot_graph);
	}
	SPDLOG_TRACE("Search tree:\n{}", search::node_to_string(*search.get_root(), true));
	if (!tree_dot_graph.empty()) {
		SPDLOG_INFO("Writing search tree to '{}'", tree_dot_graph.c_str());
		visualization::search_tree_to_graphviz(*search.get_root(), true).render_to_file(tree_dot_graph);
	}

	// controller synthesis
	SPDLOG_INFO("Creating controller");
	auto controller = controller_synthesis::create_controller(search.get_root(),
	                                                          controller_actions,
	                                                          environment_actions,
	                                                          K);
	if (!controller_dot_path.empty()) {
		SPDLOG_INFO("Writing controller to '{}'", controller_dot_path.c_str());
		visualization::ta_to_graphviz(controller, !hide_controller_labels)
		  .render_to_file(controller_dot_path);
	}
	if (!controller_proto_path.empty()) {
		SPDLOG_INFO("Writing controller proto to '{}'", controller_proto_path.c_str());
		std::ofstream fs(controller_proto_path);
		fs << automata::ta::ta_to_proto(controller).SerializeAsString();
	}
}

} // namespace tacos::app
