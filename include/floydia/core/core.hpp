#pragma once

#include <floydia/core/renderer.hpp>

namespace floyd {

struct Core final {
	Renderer renderer = Renderer();
	static inline Core& get() noexcept {
		static Core core = Core();
		return core;
	}
};

} // namespace floyd
