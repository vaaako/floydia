#pragma once

#include <floydia/core/renderer.hpp>
#include <floydia/core/assets.hpp>

namespace floyd {

// Global class agregator
struct Core final {
	// Renderer object
	std::unique_ptr<Renderer> renderer = nullptr;
	// Shader Program, Material, Texture and Mesh agregator
	std::unique_ptr<Assets> assets = nullptr;

	// Global access to Core object
	static Core& get() {
		static Core _instance; // constructed once here
		return _instance;
	}

	Core() noexcept;
	// After initialized. Core is accessible with Core::instance
	void initialize() noexcept;
};

// Global accessor
inline Assets& assets() noexcept {
	return *Core::get().assets;
}

} // namespace floyd
