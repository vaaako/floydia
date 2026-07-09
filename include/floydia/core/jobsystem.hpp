#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
namespace floyd {

// Split jobs across worker threads
class JobSystem final {
	public:
		JobSystem(const size_t workers = 0) noexcept;
		~JobSystem() noexcept;

		// Waits until every scheduled job has finished
		void wait() noexcept;
		// Returns the number of worker threads
		inline size_t workers_count() const noexcept { return this->_workers_count; }

		// Schedules a single task for execution
		void dispatch(const std::function<void()>& callback);
		// Executes a range of work in parallel. Divides the job between workers.
		// Use when each index is fully independent
		void parallel_for(const size_t begin, const size_t end, const std::function<void(size_t)>& callback);
		// Same as `parallel_for`, but callback receives (chunk_begin, chunk_end, chunk_index)
		// instead of a single index. Use when you need per-chunk state (e.g., a thread-local accumulator
		// merged after wait()), since chunk_index lets you pick the right slot
		void parallel_for_chunks(const size_t begin, const size_t end, const std::function<void(size_t, size_t, size_t)>& callback);
	
		// Returns the index of the worker calling this from a job
		static size_t worker_index() noexcept;

	private:
		// Main loop for all threads
		void worker_loop(const size_t index);

		// Threads alive
		std::vector<std::thread> workers;
		// Available jobs
		std::queue<std::function<void()>> jobs;
		// Protects 'jobs'
		std::mutex queue_mutex;
		// Working jobs
		std::atomic<size_t> pending_jobs;
		// Signals worker threads to exit their loop when the pool is being destroyed
		std::atomic<bool> stop_flag = false;

		// Notified when a job is available. Workers sleep on this CV when the job queue is empty
		std::condition_variable cv_task;
		// Notified when all jobs are finished. The main thread sleeps on this CV inside 'wait'
		std::condition_variable cv_finished;

		// Workers available
		size_t _workers_count = 0;
};

} // namespace floyd
