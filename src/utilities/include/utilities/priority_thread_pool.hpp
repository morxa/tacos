/***************************************************************************
 *  priority_thread_pool.hpp - A thread pool with a priority queue
 *
 *  Created:   Wed 10 Mar 23:06:29 CET 2021
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

#pragma once

#include "priority_thread_pool.h"

#include <mutex>

namespace utilities {

class QueueClosedException : public std::logic_error
{
	// Use base class constructors.
	using std::logic_error::logic_error;
};

class QueueStartedException : public std::logic_error
{
	// Use base class constructors.
	using std::logic_error::logic_error;
};

template <class Priority, class T>
ThreadPool<Priority, T>::ThreadPool(std::size_t size, bool start_pool) : size(size)
{
	if (start_pool) {
		start();
	}
}

template <class Priority, class T>
void
ThreadPool<Priority, T>::start()
{
	if (started) {
		throw QueueStartedException("Pool already started");
	}
	for (std::size_t i = 0; i < size; ++i) {
		workers.push_back(std::thread{[this]() {
			while (!stopping) {
				std::unique_lock lock{queue_mutex};
				while (!queue.empty()) {
					auto [priority, job] = queue.top();
					queue.pop();
					lock.unlock();
					job();
					lock.lock();
					if (stopping) {
						return;
					}
				}
				if (!queue_open) {
					return;
				}
				// Wait for the stop signal or a new job.
				queue_cond.wait(lock, [this] { return stopping || !queue.empty() || !queue_open; });
			}
		}});
	}
	started = true;
}

template <class Priority, class T>
ThreadPool<Priority, T>::~ThreadPool()
{
	stop();
}

template <class Priority, class T>
void
ThreadPool<Priority, T>::add_job(std::pair<Priority, T> &&job)
{
	if (!queue_open) {
		throw QueueClosedException("Queue is closed!");
	}
	std::lock_guard guard{queue_mutex};
	queue.push(job);
	queue_cond.notify_one();
}

template <class Priority, class T>
void
ThreadPool<Priority, T>::add_job(T &&job, const Priority &priority)
{
	add_job(std::make_pair(priority, job));
}

template <class Priority, class T>
void
ThreadPool<Priority, T>::close_queue()
{
	queue_open = false;
}

template <class Priority, class T>
void
ThreadPool<Priority, T>::finish()
{
	close_queue();
	{
		std::lock_guard guard{queue_mutex};
		queue_cond.notify_all();
	}
	for (auto &worker : workers) {
		if (worker.joinable()) {
			worker.join();
		}
	}
}

template <class Priority, class T>
void
ThreadPool<Priority, T>::stop()
{
	stopping = true;
	finish();
}

} // namespace utilities
