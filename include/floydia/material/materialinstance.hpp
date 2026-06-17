#pragma once

#include <floydia/material/material.hpp>
#include <functional>

namespace floyd {

// Material is shared and immutable GPU state.
// MaterialInstance is overrides per-object.
// This allows user to change states and keep Material immutable.
// Different instances may result in separate batches
struct MaterialInstance {
	std::shared_ptr<Material> base;
	std::shared_ptr<Texture> albedo;
	// Metallic controls specular color.
	// Metal (1.0) reflects object color, plastic (0.0) reflects white
	float metallic = 0.0f;
	// Roughness controlls shininess.
	// Low roughness = sharp highlight
	float roughness = 1.0f;

	// Optional. Called at end of bind()
	std::function<void()> on_bind;

	inline MaterialInstance(const std::shared_ptr<Material>& base, const std::shared_ptr<Texture> albedo)
		: base(base), albedo(albedo) {}

	inline void bind() const noexcept {
		this->albedo->bind(0);
		if(this->on_bind) this->on_bind();
		// NOTE: 'metallic' and 'roughness' are per batch, not instance.
		// All objects on the same batch share the same MaterialInstance,
		// so the valor is constant
		//
		// NOTE: for shaders that don't have these uniforms, it will simply ignore
	}
};

} // namespace floyd
