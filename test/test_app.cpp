/***************************************************************************
 *  test_app.cpp - Test the main application
 *
 *  Created:   Tue 20 Apr 19:57:43 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "app/app.h"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>

static const std::filesystem::path test_data_dir{TEST_DATA_DIR};

TEST_CASE("Launch the main application", "[app]")
{
	const std::filesystem::path test_scenario_dir     = test_data_dir / "simple";
	const std::filesystem::path plant_path            = test_scenario_dir / "plant.pbtxt";
	const std::filesystem::path spec_path             = test_scenario_dir / "spec.pbtxt";
	const std::filesystem::path controller_dot_path   = test_scenario_dir / "controller.png";
	const std::filesystem::path controller_proto_path = test_scenario_dir / "controller.pbtxt";
	const std::filesystem::path plant_dot_graph       = test_scenario_dir / "plant.png";
	const std::filesystem::path tree_dot_graph        = test_scenario_dir / "tree.png";
	SECTION("Simple launch")
	{
		const std::array argv{
		  "app",
		  "--plant",
		  plant_path.c_str(),
		  "--spec",
		  spec_path.c_str(),
		  "-c",
		  "c",
		};
		tacos::app::Launcher launcher{argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
	}
	SECTION("Single-threaded run")
	{
		const std::array argv{
		  "app",
		  "--single-threaded",
		  "--plant",
		  plant_path.c_str(),
		  "--spec",
		  spec_path.c_str(),
		  "-c",
		  "c",
		};
		tacos::app::Launcher launcher{argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
	}
	SECTION("Select heuristics")
	{
		for (const auto &heuristic : {"bfs", "dfs", "composite", "random", "time"}) {
			const std::array argv{
			  "app",
			  "--single-threaded",
			  "--plant",
			  plant_path.c_str(),
			  "--spec",
			  spec_path.c_str(),
			  "-c",
			  "c",
			  "--heuristic",
			  heuristic,
			};
			tacos::app::Launcher launcher{argv.size(), argv.data()};
			CHECK_NOTHROW(launcher.run());
		}
	}
	SECTION("Visualizations")
	{
		const std::array argv{
		  "app",
		  "--plant",
		  plant_path.c_str(),
		  "--spec",
		  spec_path.c_str(),
		  "-c",
		  "c",
		  "--visualize-plant",
		  plant_dot_graph.c_str(),
		  "--visualize-controller",
		  controller_dot_path.c_str(),
		  "--hide-controller-labels",
		  "--visualize-search-tree",
		  tree_dot_graph.c_str(),
		};
		tacos::app::Launcher launcher{argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
		CHECK(std::filesystem::exists(plant_dot_graph));
		std::filesystem::remove(plant_dot_graph);
		CHECK(std::filesystem::exists(controller_dot_path));
		std::filesystem::remove(controller_dot_path);
		CHECK(std::filesystem::exists(tree_dot_graph));
		std::filesystem::remove(tree_dot_graph);
	}
	SECTION("Create controller proto")
	{
		const std::array argv{
		  "app",
		  "--plant",
		  plant_path.c_str(),
		  "--spec",
		  spec_path.c_str(),
		  "-c",
		  "c",
		  "-o",
		  controller_proto_path.c_str(),
		};
		tacos::app::Launcher launcher{argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
		CHECK(std::filesystem::exists(controller_proto_path));
		std::filesystem::remove(controller_proto_path);
	}
}

TEST_CASE("Running the app with invalid input", "[app]")
{
	{
		constexpr int     argc       = 2;
		const char *const argv[argc] = {"app", "--help"};
		// Showing the help should not throw.
		CHECK_NOTHROW(tacos::app::Launcher{argc, argv}.run());
	}
	{
		const char *const argv[1] = {"app"};
		CHECK_THROWS(tacos::app::Launcher{1, argv});
	}
	{
		constexpr int     argc       = 7;
		const char *const argv[argc] = {
		  "app",
		  "--plant",
		  "nonexistent"
		  "--spec",
		  "nonexistent"
		  "-c",
		  "c",
		};
		CHECK_THROWS(tacos::app::Launcher{argc, argv});
	}
	{
		const std::filesystem::path test_scenario_dir = test_data_dir / "simple";
		const std::filesystem::path plant_path        = test_scenario_dir / "plant.pbtxt";
		const std::filesystem::path spec_path         = test_scenario_dir / "spec.pbtxt";
		constexpr const int         argc              = 9;
		// Arguments are switched.
		const char *const argv[argc] = {
		  "app", "--plant", spec_path.c_str(), "--spec", plant_path.c_str(), "-c", "c"};
		CHECK_THROWS(tacos::app::Launcher{argc, argv});
	}
}
