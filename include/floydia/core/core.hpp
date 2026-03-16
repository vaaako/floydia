#pragma once

#include <floydia/core/renderer.hpp>

namespace floyd {

class Core final {
	public:
		Renderer renderer = Renderer();
		static inline Core& get() noexcept {
			static Core core = Core();
			return core;
		}
};

} // namespace floyd
