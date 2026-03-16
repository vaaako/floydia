#pragma once

#include <floydia/core/renderer.hpp>

namespace floyd {

class Core final {
	public:
		Core() = default;

		static inline Core& get() noexcept {
			static Core core = Core();
			return core;
		}

		Renderer renderer = Renderer();
		// InputSystem inputsystem = InputSystem();
};

} // namespace floyd
