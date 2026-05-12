#pragma once

#include <floydia/material/material.hpp>

namespace floyd {

// Material is shared and immutable GPU state.
// MaterialInstance is overrides per-object.
// This allows user to change states and keep Material immutable.
// Different instances may result in separate batches
struct MaterialInstance {
	std::shared_ptr<Material> base;
	std::shared_ptr<Texture> albedo;
	// float metallic = 0.0f;
	// float roughness = 1.0f;

	inline MaterialInstance(const std::shared_ptr<Material>& base, const std::shared_ptr<Texture> albedo)
		: base(base), albedo(albedo) {}

	inline void bind() const noexcept {
		this->albedo->bind(0);
	}
};

} // namespace floyd
