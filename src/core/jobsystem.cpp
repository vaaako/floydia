#include "floydia/core/jobsystem.hpp"
#include <algorithm>
#include <mutex>

namespace floyd {

JobSystem::JobSystem(const size_t workers) noexcept {
	this->_workers_count = std::max<size_t>(1,
		(workers != 0) ? workers : std::thread::hardware_concurrency() - 1); // -1: Leave one for main thread
	this->workers.reserve(this->_workers_count);

	// Make all threads now because is expensive to open a new thread
	for(size_t i = 0; i < this->_workers_count; ++i) {
		this->workers.emplace_back([this] { this->worker_loop(); });
	}
}

JobSystem::~JobSystem() noexcept {
	this->stop_flag.store(true, std::memory_order_release); // release: write. Make visible any write before to any thread reading after this
	this->cv_task.notify_all(); // Wake all workers on 'cv_task.wait' inside 'worker_loop'

	// Stop all threads
	for(std::thread& t : workers) {
		if(t.joinable()) t.join();
	}
}

void JobSystem::wait() noexcept {
	// 'std::unique_lock' because 'wait' needs to unlock and lock internally
	std::unique_lock<std::mutex> lock = std::unique_lock<std::mutex>(this->queue_mutex);
	// The caller sleeps here
	this->cv_finished.wait(lock, [this] {
		// acquire: read. grants that if see 'stop_flag == true' (set by destructor)
		// also see any write the destructor did before this
		return this->pending_jobs.load(std::memory_order_acquire) == 0;
	});
}

void JobSystem::worker_loop() {
	// Execute jobs as it comes
	while(true) {
		std::function<void()> job;

		// Block to lock and unlock 'queue_mutex'
		{
			std::unique_lock<std::mutex> lock = std::unique_lock<std::mutex>(this->queue_mutex);

			// The OS sleeps this thread until the condition is true (which is checked when 'cv_task' is notified)
			this->cv_task.wait(lock, [this]() {
				return this->stop_flag.load(std::memory_order_acquire) || !this->jobs.empty();
			});

			// Shutting down and no jobs left
			if(this->stop_flag.load(std::memory_order_acquire) && this->jobs.empty()) return;
		
			job = std::move(this->jobs.front());
			this->jobs.pop();
		}
		job(); // Execute job

		// Subtract '1' from 'pending_jobs' and returns value before subtraction
		if(this->pending_jobs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
			std::lock_guard<std::mutex> lock = std::lock_guard<std::mutex>(this->queue_mutex);
			this->cv_finished.notify_one(); // Last pending job, notify main thread on 'wait'
		}
	}
}

void JobSystem::dispatch(const std::function<void()>& callback) {
	// Increment working jobs. Need to be before pushing,
	// because there is a small chance of a worker getting a job and decrementing first, then incrementing (if -1, makes a overflow)
	this->pending_jobs.fetch_add(1, std::memory_order_acq_rel);
	{
		std::lock_guard<std::mutex> lock = std::lock_guard<std::mutex>(this->queue_mutex); // Simple lock
		this->jobs.push(callback);
	}
	this->cv_task.notify_one(); // Notify a single worker on 'cv_task.wait'
}

void JobSystem::parallel_for(const size_t begin, const size_t end, const std::function<void(size_t)>& callback) {
	if(begin >= end) return;

	const size_t range = end - begin;
	const size_t chunk_count = std::min(range, this->_workers_count);
	const size_t chunk_size = (range + chunk_count - 1) / chunk_count; // Divide and round up ('std::ceil' wont work here, because of how it works internally)

	for(size_t c = 0; c < chunk_count; ++c) {
		const size_t chunk_begin = begin + c * chunk_size;
		const size_t chunk_end = std::min(chunk_begin + chunk_size, end); // std::min: May overflow 'end'
		if(chunk_begin >= chunk_end) continue; // Just in case

		this->dispatch([chunk_begin, chunk_end, &callback]() {
			for(size_t i = chunk_begin; i < chunk_end; ++i) callback(i);
		});
	}

	this->wait();
}

} // namespace floyd
