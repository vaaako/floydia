#include <floydia/time/clock.hpp>
#include <algorithm>

namespace floyd {

Clock::Clock() noexcept : last_time(HighResClock::now()) {}

float Clock::mark() noexcept {
	const std::chrono::time_point now = HighResClock::now();
	const std::chrono::duration<float> delta = now - this->last_time;

	this->last_time = now;
	this->_delta = std::min(delta.count(), 0.1f); // this avoids spikes at the calculation start/end
	// change 0.1 to 100.0f for seconds
	return this->_delta;
}

bool Clock::every(const float interval) noexcept {
	this->elapsed += this->_delta;
	if(this->elapsed >= interval) {
		this->elapsed = 0.0f;
		return true;
	}
	return false;
}

} // namespace floyd
