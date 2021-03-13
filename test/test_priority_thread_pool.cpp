/***************************************************************************
 *  test_priority_thread_pool.cpp - Test the priority thread pool
 *
 *  Created:   Thu 11 Mar 09:31:55 CET 2021
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

#include "utilities/priority_thread_pool.h"
#include "utilities/priority_thread_pool.hpp"

#include <catch2/catch.hpp>
#include <chrono>
#include <mutex>
#include <set>
#include <thread>

using utilities::ThreadPool;

TEST_CASE("Create and run a priority thread pool", "[threading]")
{
	std::set<int> res;
	std::mutex    res_mutex;
	ThreadPool    pool{};
	SECTION("Starting some simple jobs")
	{
		for (int i = 0; i < 10; ++i) {
			pool.add_job(std::make_pair(i, [&res_mutex, &res, i]() {
				std::lock_guard<std::mutex> guard{res_mutex};
				res.insert(i);
			}));
		}
		pool.finish();
		CHECK(res == std::set{0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
	}
	SECTION("Exception occurs when pushing to a closed queue")
	{
		pool.close_queue();
		CHECK_THROWS_AS(pool.add_job(std::make_pair(0, [] {})), utilities::QueueClosedException);
	}
	SECTION("Jobs are canceled after stopping the queue")
	{
		constexpr int num_jobs = 100;
		for (int i = 0; i < num_jobs; ++i) {
			pool.add_job(std::make_pair(i, [&res_mutex, &res, i]() {
				std::this_thread::sleep_for(std::chrono::milliseconds{100});
				std::lock_guard<std::mutex> guard{res_mutex};
				res.insert(i);
			}));
		}
		pool.cancel();
		CHECK(res.size() < num_jobs);
	}
	SECTION("Two identical jobs are executed twice")
	{
		std::vector<int> res_vec;
		for (int i = 0; i < 2; ++i) {
			pool.add_job(std::make_pair(42, [&res_mutex, &res_vec]() {
				std::lock_guard<std::mutex> guard{res_mutex};
				res_vec.push_back(42);
			}));
		}
		pool.finish();
		CHECK(res_vec == std::vector{42, 42});
	}
	SECTION("Add job with default priority")
	{
		pool.add_job([&res_mutex, &res]() {
			std::lock_guard<std::mutex> guard{res_mutex};
			res.insert(1);
		});
		pool.finish();
		CHECK(res == std::set{1});
	}
}
