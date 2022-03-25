/***************************************************************************
 *  ta_visualizer.cpp - Tool to visualize a TA
 *
 *  Created:   Thu 22 Apr 19:13:45 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include "app/app.h"
#include "automata/ta.pb.h"
#include "automata/ta_proto.h"
#include "visualization/ta_to_graphviz.h"

#include <spdlog/spdlog.h>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

int
main(int argc, const char *const argv[])
{
	using boost::program_options::value;
	boost::program_options::options_description options("Allowed options");
	std::filesystem::path                       proto_file;
	std::filesystem::path                       output_file;
	// clang-format off
  options.add_options()
    ("help,h", "Show help")
    ("proto-file", value(&proto_file)->required(), "Path to the pbtxt to visualize")
    ("output-file",value(&output_file)->required(), "Output path")
  ;
	// clang-format on
	boost::program_options::positional_options_description pos_options;
	pos_options.add("proto-file", 1);
	pos_options.add("output-file", 1);
	boost::program_options::variables_map variables;
	boost::program_options::store(boost::program_options::command_line_parser(argc, argv)
	                                .options(options)
	                                .positional(pos_options)
	                                .run(),
	                              variables);
	if (variables.count("help")) {
		std::cout << options;
		return 0;
	}
	boost::program_options::notify(variables);
	tacos::automata::ta::proto::TimedAutomaton ta_proto;
	tacos::app::read_proto_from_file(proto_file, &ta_proto);
	tacos::visualization::ta_to_graphviz(tacos::automata::ta::parse_proto(ta_proto))
	  .render_to_file(output_file);
}
