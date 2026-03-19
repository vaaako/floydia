#pragma once

#include <chrono>

namespace floyd {

// Flexible clock that provides delta time
class Clock {
	public:
		using HighResClock = std::chrono::high_resolution_clock;

		Clock() noexcept;

		// Returns elapsed time since last mark
		float mark() noexcept;

		// Returns current elapsed time, even if the clock is running
		inline float delta() const noexcept {
			return this->_delta;
		}

		// Resets the clock
		inline void reset() noexcept {
			this->last_time = HighResClock::now();
		}

	private:
		HighResClock::time_point last_time;
		float _delta;
};

} // namespace floyd
