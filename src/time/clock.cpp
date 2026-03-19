#include <floydia/time/clock.hpp>
#include <algorithm>

namespace floyd {

Clock::Clock() noexcept : last_time(HighResClock::now()) {}

float Clock::mark() noexcept {
	const std::chrono::time_point now = HighResClock::now();
	const std::chrono::duration<float> delta = now - this->last_time;

	this->last_time = now;
	this->_delta = std::min(delta.count(), 0.1f); // this avoids spikes at the calculation start/end
	return this->_delta;
}

} // namespace floyd
