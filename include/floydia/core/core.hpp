#pragma once

#include "floydia/core/assets.hpp"
#include "floydia/core/jobsystem.hpp"
#include <cstddef>
#include <memory>

namespace floyd {

// Global class agregator
struct Core final {
	// Set this before initializing window. '0' is the default for "all available"
	static size_t thread_count;

	// Shader Program, Material, Texture and Mesh agregator
	std::unique_ptr<Assets> assets = nullptr;
	// Multithread manager
	std::unique_ptr<JobSystem> jobsystem = nullptr;
	
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

inline JobSystem& jobsystem() noexcept {
	return *Core::get().jobsystem;
}

} // namespace floyd
