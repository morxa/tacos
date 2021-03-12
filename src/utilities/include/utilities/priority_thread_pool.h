/***************************************************************************
 *  priority_thread_pool.h - A thread pool with a priority queue
 *
 *  Created:   Wed 10 Mar 22:57:22 CET 2021
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

#ifndef SRC_UTILITIES_INCLUDE_UTILITIES_PRIORITY_THREAD_POOL_H
#define SRC_UTILITIES_INCLUDE_UTILITIES_PRIORITY_THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

namespace utilities {

template <typename T>
struct CompareFirstOfPair
{
	bool
	operator()(const T &t1, const T &t2) const
	{
		return t1.first < t2.first;
	}
};

template <class Priority = int, class T = std::function<void()>>
class ThreadPool
{
public:
	ThreadPool(std::size_t num_threads = std::thread::hardware_concurrency(), bool start = true);
	~ThreadPool();
	void add_job(std::pair<Priority, T> &&job);
	void add_job(T &&job, const Priority &priority = Priority{});
	void start();
	void stop();
	void close_queue();
	void finish();

private:
	std::size_t              size;
	bool                     started{false};
	std::vector<std::thread> workers;
	std::priority_queue<std::pair<Priority, T>,
	                    std::vector<std::pair<Priority, T>>,
	                    CompareFirstOfPair<std::pair<Priority, T>>>
	                        queue;
	std::atomic_bool        stopping{false};
	std::atomic_bool        queue_open{true};
	std::mutex              queue_mutex;
	std::condition_variable queue_cond;
};

} // namespace utilities

#include "priority_thread_pool.hpp"

#endif /* ifndef SRC_UTILITIES_INCLUDE_UTILITIES_PRIORITY_THREAD_POOL_H */
