/***************************************************************************
 *  test_golog_app.cpp - Test the golog application
 *
 *  Created:   Wed 11 Jan 14:11:00 CEST 2023
 *  Copyright  2023  Daniel Swoboda <swoboda@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "golog_app/app.h"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>

static const std::filesystem::path test_data_dir{TEST_DATA_DIR};
const std::filesystem::path        test_scenario_dir = test_data_dir / "golog_app";
const std::filesystem::path        plant_path        = test_scenario_dir / "robot_camera.gpp";
const std::filesystem::path        spec_path = test_scenario_dir / "robot_camera_spec.pbtxt";
const std::filesystem::path spec_pass_path   = test_scenario_dir / "robot_camera_spec_pass.pbtxt";

TEST_CASE("Launch the Golog app in controller synthesis mode", "[app]")
{
	const std::filesystem::path controller_dot_path   = test_scenario_dir / "controller.png";
	const std::filesystem::path controller_proto_path = test_scenario_dir / "controller.pbtxt";
	const std::filesystem::path tree_dot_graph        = test_scenario_dir / "tree.png";

	std::vector<const char *> basic_parameters{"gocos",
	                                           "--program",
	                                           plant_path.c_str(),
	                                           "--spec",
	                                           spec_path.c_str(),
	                                           "-k",
	                                           "1",
	                                           "-c",
	                                           "start(drive(machine1, machine2))",
	                                           "-c",
	                                           "start(grasp(machine2, obj1))",
	                                           "-c",
	                                           "start(boot_camera())",
	                                           "-c",
	                                           "start(shutdown_camera())",
	                                           "-e",
	                                           "end(drive(machine1, machine2))",
	                                           "-e",
	                                           "end(grasp(machine2, obj1))",
	                                           "-e",
	                                           "end(boot_camera())",
	                                           "-e",
	                                           "end(shutdown_camera())"};

	SECTION("Simple Launch")
	{
		std::vector<const char *>  argv(basic_parameters);
		tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
	}

	SECTION("Debug Launch")
	{
		std::vector<const char *> argv(basic_parameters);
		std::vector<const char *> debug_param{"--debug"};
		argv.insert(argv.end(), debug_param.begin(), debug_param.end());
		tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
	}

	SECTION("Select heuristics")
	{
		for (const auto &heuristic : {"dfs"}) {
			std::vector<const char *> argv(basic_parameters);
			std::vector<const char *> heuristic_params{"--heuristic", heuristic};
			argv.insert(argv.end(), heuristic_params.begin(), heuristic_params.end());

			tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
			CHECK_NOTHROW(launcher.run());
		}
	}

	SECTION("Visualizations")
	{
		std::vector<const char *> argv(basic_parameters);
		std::vector<const char *> visualize_params{"--visualize-controller",
		                                           controller_dot_path.c_str(),
		                                           "--hide-controller-labels",
		                                           "--visualize-search-tree",
		                                           tree_dot_graph.c_str()};
		argv.insert(argv.end(), visualize_params.begin(), visualize_params.end());
		tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
		CHECK(std::filesystem::exists(controller_dot_path));
		std::filesystem::remove(controller_dot_path);
		CHECK(std::filesystem::exists(tree_dot_graph));
		std::filesystem::remove(tree_dot_graph);
	}

	SECTION("Create controller proto")
	{
		std::vector<const char *> argv(basic_parameters);
		std::vector<const char *> controller_params{"-o", controller_proto_path.c_str()};
		argv.insert(argv.end(), controller_params.begin(), controller_params.end());
		tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
		CHECK(std::filesystem::exists(controller_proto_path));
		std::filesystem::remove(controller_proto_path);
	}
}

TEST_CASE("Launch the Golog app in verification mode", "[app]")
{
	const std::filesystem::path ce_dot_graph   = test_scenario_dir / "ce.png";
	const std::filesystem::path tree_dot_graph = test_scenario_dir / "tree.png";

	std::vector<const char *> basic_parameters{"gocos",
	                                           "--program",
	                                           plant_path.c_str(),
	                                           "--spec",
	                                           spec_path.c_str(),
	                                           "-v",
	                                           "-k",
	                                           "1",
	                                           "-e",
	                                           "start(drive(machine1, machine2))",
	                                           "-e",
	                                           "start(grasp(machine2, obj1))",
	                                           "-e",
	                                           "start(boot_camera())",
	                                           "-e",
	                                           "start(shutdown_camera())",
	                                           "-e",
	                                           "end(drive(machine1, machine2))",
	                                           "-e",
	                                           "end(grasp(machine2, obj1))",
	                                           "-e",
	                                           "end(boot_camera())",
	                                           "-e",
	                                           "end(shutdown_camera())"};

	std::vector<const char *> pass_parameters{
	  "gocos", "--program", plant_path.c_str(), "--spec", spec_pass_path.c_str(), "-v", "-k", "1"};

	SECTION("Simple Launch")
	{
		std::vector<const char *>  argv(basic_parameters);
		tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
	}

	SECTION("Debug Launch")
	{
		std::vector<const char *> argv(basic_parameters);
		std::vector<const char *> debug_param{"--debug"};
		argv.insert(argv.end(), debug_param.begin(), debug_param.end());
		tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
	}

	SECTION("Select heuristics")
	{
		for (const auto &heuristic : {"dfs"}) {
			std::vector<const char *> argv(basic_parameters);
			std::vector<const char *> heuristic_params{"--heuristic", heuristic};
			argv.insert(argv.end(), heuristic_params.begin(), heuristic_params.end());

			tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
			CHECK_NOTHROW(launcher.run());
		}
	}

	SECTION("Visualizations")
	{
		std::vector<const char *> argv(basic_parameters);
		std::vector<const char *> visualize_params{"--visualize-counter-example",
		                                           ce_dot_graph.c_str(),
		                                           "--hide-controller-labels",
		                                           "--visualize-search-tree",
		                                           tree_dot_graph.c_str()};
		argv.insert(argv.end(), visualize_params.begin(), visualize_params.end());
		tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
		CHECK(std::filesystem::exists(ce_dot_graph));
		std::filesystem::remove(ce_dot_graph);
		CHECK(std::filesystem::exists(tree_dot_graph));
		std::filesystem::remove(tree_dot_graph);
	}

	SECTION("Verification Pass")
	{
		std::vector<const char *> argv(pass_parameters);
		std::vector<const char *> visualize_params{"--visualize-counter-example",
		                                           ce_dot_graph.c_str(),
		                                           "--hide-controller-labels"};
		argv.insert(argv.end(), visualize_params.begin(), visualize_params.end());
		tacos::golog_app::Launcher launcher{(int)argv.size(), argv.data()};
		CHECK_NOTHROW(launcher.run());
		CHECK(!std::filesystem::exists(ce_dot_graph));
		std::filesystem::remove(ce_dot_graph);
	}
}

TEST_CASE("Running the golog app with invalid input", "[app]")
{
	{
		constexpr int     argc       = 2;
		const char *const argv[argc] = {"gocos", "--help"};
		// Showing the help should not throw.
		CHECK_NOTHROW(tacos::golog_app::Launcher{argc, argv}.run());
	}
	{
		// No arguments
		const char *const argv[1] = {"gocos"};
		CHECK_THROWS(tacos::golog_app::Launcher{1, argv}.run());
	}
	{
		// Spec file does not exist
		constexpr int     argc       = 9;
		const char *const argv[argc] = {
		  "gocos", "-p", plant_path.c_str(), "-s", "nonexistent", "-c", "c", "-k", "1"};
		CHECK_THROWS(tacos::golog_app::Launcher{argc, argv}.run());
	}
	{
		// Spec file eixsts but is not a valid pbtxt file for MTL specs
		constexpr int     argc       = 9;
		const char *const argv[argc] = {
		  "gocos", "-p", plant_path.c_str(), "-s", plant_path.c_str(), "-c", "c", "-k", "1"};
		CHECK_THROWS(tacos::golog_app::Launcher{argc, argv}.run());
	}
	{
		// Program file not existing
		constexpr int     argc       = 9;
		const char *const argv[argc] = {
		  "gocos", "-p", "nonexistent", "-s", spec_path.c_str(), "-c", "c", "-k", "1"};
		CHECK_THROWS(tacos::golog_app::Launcher{argc, argv}.run());
	}
	{
		// Arguments are switched.
		const char *const argv[9] = {
		  "gocos", "-p", spec_path.c_str(), "-s", plant_path.c_str(), "-c", "c", "-k", "1"};
		CHECK_THROWS(tacos::golog_app::Launcher{9, argv}.run());
	}
	{
		// Unknown heuristic
		const char *const argv[11] = {"gocos",
		                              "-p",
		                              plant_path.c_str(),
		                              "-s",
		                              spec_path.c_str(),
		                              "-c",
		                              "c",
		                              "--heuristic",
		                              "NONE",
		                              "-k",
		                              "1"};
		CHECK_THROWS(tacos::golog_app::Launcher{11, argv}.run());
	}
}
