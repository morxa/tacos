/***************************************************************************
 *  app.h - The tool's main binary
 *
 *  Created:   Mon 19 Apr 15:29:55 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
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

#include <google/protobuf/message.h>

#include <filesystem>

namespace app {

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
	void read_proto_from_file(const std::filesystem::path &path, google::protobuf::Message *output);

	std::filesystem::path plant_path;
	std::filesystem::path specification_path;
	std::filesystem::path controller_dot_path;
	std::filesystem::path controller_proto_path;
	std::filesystem::path plant_dot_graph;
	std::filesystem::path tree_dot_graph;
	bool                  show_help{false};
	bool                  multi_threaded{true};
	std::set<std::string> controller_actions;
	std::string           heuristic;
};

} // namespace app
