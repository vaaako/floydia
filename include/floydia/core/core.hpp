#pragma once

#include <floydia/core/assets.hpp>
#include <floydia/core/renderer.hpp>
#include <floydia/core/buffermanager.hpp>

namespace floyd {

// https://github.com/ColleagueRiley/RGFW/blob/main/examples/multi-window/multi-window.c

// Global class agregator
struct Core final {
	// Global access to Core object
	static inline Core* instance = nullptr;
	// Global access to Core object
	static Core& get() {
		static Core _instance; // constructed once here
		return _instance;
	}

	// Shader Program manager
	BufferManager buffermanager;
	// Renderer object
	std::unique_ptr<Renderer> renderer;
	// Shader Program, Material, Texture and Mesh agregator
	std::unique_ptr<Assets> assets;

	Core() noexcept;
	// After initialized. Core is accessible with Core::instance
	void initialize() noexcept;
	inline ~Core() noexcept { instance = nullptr; }

	private:
		bool initialized = false;
};

} // namespace floyd
