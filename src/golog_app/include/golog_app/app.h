/***************************************************************************
 *  app.h - The tool's main app library of the GOLOG variety
 *
 *  Created:   Tue 20 Nov 18:40:33 CEST 2022
 *  Copyright  2022  Daniel Swoboda <swoboda@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "gocos/golog_program.h"
#include "mtl/MTLFormula.h"

#include <google/protobuf/message.h>

#include <filesystem>

namespace tacos::golog_app {

/** @brief Launcher for the main application.
 * The launcher runs the main application, reads the input from pbtxt files, runs the search, and
 * finally generates a controller.*/
class Launcher
{
public:
	/** Initialize the launcher with the given command line arguments.
	 * @param argc The number of arguments, as passed to main()
	 * @param argv The arguments, as passed to main()
	 */
	Launcher(int argc, const char *const argv[]);

	/** Run the launcher. */
	void run();

private:
	void parse_command_line(int argc, const char *const argv[]);

	std::filesystem::path program_path;
	std::filesystem::path specification_path;
	unsigned int          K;
	std::filesystem::path controller_dot_path;
	std::filesystem::path controller_proto_path;
	std::filesystem::path tree_dot_graph;
	bool                  show_help{false};
	bool                  multi_threaded{true};
	bool                  debug{false};
	bool                  hide_controller_labels{false};
	std::set<std::string> controller_actions;
	std::set<std::string> environment_actions;
	std::string           heuristic;
};

void read_proto_from_file(const std::filesystem::path &path, google::protobuf::Message *output);
search::GologProgram read_golog_from_file(const std::filesystem::path &path,
                                          std::set<std::string>       &relevant_fluents);
std::set<std::string>
unwrap_fluents(std::set<logic::AtomicProposition<std::set<std::string>>> input);

} // namespace tacos::golog_app
