#pragma once

#include <floydia/core/assets.hpp>
#include <floydia/core/renderer.hpp>
#include <floydia/core/buffermanager.hpp>

namespace floyd {

// Global class agregator
struct Core final {
	// Global access to Core object
	static inline Core* instance = nullptr;

	// Shader Program and Texture (TODO) manager
	BufferManager buffermanager;
	// Shader Program, Material, Texture and Mesh agregator
	Assets assets;
	// Renderer object
	Renderer renderer;

	inline Core()  noexcept
		: buffermanager(), assets(this->buffermanager), renderer()
	{
		this->instance = this;
	}

	inline ~Core() noexcept { instance = nullptr; }
};

} // namespace floyd
