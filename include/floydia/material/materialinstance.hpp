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
	// Metallic controls specular color.
	// Metal (1.0) reflects object color, plastic (0.0) reflects white
	float metallic = 0.0f;
	// Roughness controlls shininess.
	// Low roughness = sharp highlight
	float roughness = 1.0f;

	inline MaterialInstance(const std::shared_ptr<Material>& base, const std::shared_ptr<Texture> albedo)
		: base(base), albedo(albedo) {}

	inline void bind() const noexcept {
		this->albedo->bind(0);
		glProgramUniform1f(this->base->fragment->id(), 0, this->metallic);
		glProgramUniform1f(this->base->fragment->id(), 1, this->roughness);
		// NOTE: 'metallic' and 'roughness' are per batch, not instance.
		// All objects on the same batch share the same MaterialInstance,
		// so the valor is constant
	}
};

} // namespace floyd
