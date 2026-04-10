#pragma once

#include <chrono>

namespace floyd {

// Flexible clock that provides delta time
class Clock {
	public:
		using HighResClock = std::chrono::high_resolution_clock;

		Clock() noexcept;

		// Resets the timer and returns the elapsed time (in seconds) since the last call
		float mark() noexcept;
		// Returns true every 'interval' seconds, resets automatically.
		// Must be called once per frame to accumulate time correctly
		bool every(const float interval) noexcept;
		// Returns the elapsed time (in seconds) between the last two mark() calls
		inline float delta() const noexcept { return this->_delta; }
		// Resets the counting
		inline void reset() noexcept { this->last_time = HighResClock::now(); }

	private:
		HighResClock::time_point last_time;
		float _delta = 0.0f;
		float elapsed = 0.0f;
};

} // namespace floyd
