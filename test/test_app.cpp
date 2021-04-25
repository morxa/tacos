/***************************************************************************
 *  test_app.cpp - Test the main application
 *
 *  Created:   Tue 20 Apr 19:57:43 CEST 2021
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

#include "app/app.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

TEST_CASE("Launch the main application", "[.railroad][app]")
{
	constexpr const int         argc          = 19;
	const std::filesystem::path test_data_dir = std::filesystem::current_path() / "data" / "railroad";
	const std::filesystem::path plant_path    = test_data_dir / "plant.pbtxt";
	const std::filesystem::path spec_path     = test_data_dir / "spec.pbtxt";
	const std::filesystem::path controller_dot_path   = test_data_dir / "controller.png";
	const std::filesystem::path controller_proto_path = test_data_dir / "controller.pbtxt";
	const std::filesystem::path plant_dot_graph       = test_data_dir / "plant.png";
	const std::filesystem::path tree_dot_graph        = test_data_dir / "tree.png";
	const std::array<const char *, argc> argv{"app",
	                                          "--single-threaded",
	                                          "--plant",
	                                          plant_path.c_str(),
	                                          "--spec",
	                                          spec_path.c_str(),
	                                          "-c",
	                                          "start_open",
	                                          "-c",
	                                          "start_close",
	                                          "--visualize-plant",
	                                          plant_dot_graph.c_str(),
	                                          "--visualize-search-tree",
	                                          tree_dot_graph.c_str(),
	                                          "--visualize-controller",
	                                          controller_dot_path.c_str(),
	                                          "--hide-controller-labels",
	                                          "-o",
	                                          controller_proto_path.c_str()};
	app::Launcher                        launcher{argc, argv.data()};
	launcher.run();
	CHECK(std::filesystem::exists(controller_dot_path));
	std::filesystem::remove(controller_dot_path);
	CHECK(std::filesystem::exists(controller_proto_path));
	std::filesystem::remove(controller_proto_path);
	CHECK(std::filesystem::exists(plant_dot_graph));
	std::filesystem::remove(plant_dot_graph);
	CHECK(std::filesystem::exists(tree_dot_graph));
	std::filesystem::remove(tree_dot_graph);
}

TEST_CASE("Running the app with invalid input", "[app]")
{
	{
		constexpr int     argc       = 2;
		const char *const argv[argc] = {"app", "--help"};
		// Showing the help should not throw.
		CHECK_NOTHROW(app::Launcher{argc, argv}.run());
	}
	{
		const char *const argv[1] = {"app"};
		CHECK_THROWS(app::Launcher{1, argv});
	}
	{
		constexpr int     argc       = 9;
		const char *const argv[argc] = {"app",
		                                "--plant",
		                                "nonexistent"
		                                "--spec",
		                                "nonexistent"
		                                "-c",
		                                "start_open",
		                                "-c",
		                                "start_close"};
		CHECK_THROWS(app::Launcher{argc, argv});
	}
	{
		constexpr const int         argc = 11;
		const std::filesystem::path test_data_dir =
		  std::filesystem::current_path() / "data" / "railroad";
		const std::filesystem::path plant_path = test_data_dir / "plant.pbtxt";
		const std::filesystem::path spec_path  = test_data_dir / "spec.pbtxt";
		// Arguments are switched.
		const char *const argv[argc] = {"app",
		                                "--plant",
		                                spec_path.c_str(),
		                                "--spec",
		                                plant_path.c_str(),
		                                "-c",
		                                "start_open",
		                                "-c",
		                                "start_close"};
		CHECK_THROWS(app::Launcher{argc, argv});
	}
}
