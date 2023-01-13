/***************************************************************************
 *  app.h - The tool's main binary
 *
 *  Created:   Mon 19 Apr 15:29:55 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include <google/protobuf/message.h>

#include <filesystem>

/// The main application
namespace tacos::app {

/** @brief Launcher for the main application.
 *
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

	std::filesystem::path plant_path;
	std::filesystem::path specification_path;
	std::filesystem::path controller_dot_path;
	std::filesystem::path controller_proto_path;
	std::filesystem::path plant_dot_graph;
	std::filesystem::path tree_dot_graph;
	bool                  show_help{false};
	bool                  multi_threaded{true};
	bool                  debug{false};
	bool                  hide_controller_labels{false};
	std::set<std::string> controller_actions;
	std::string           heuristic;
};

/** @brief Read a protobuf message from a file.
 *
 * @param path The path to the file to read.
 * @param output A pointer to the protobuf message to write the proto to.
 */
void read_proto_from_file(const std::filesystem::path &path, google::protobuf::Message *output);

} // namespace tacos::app
