#include <floydia/gfx/material.hpp>

namespace floyd {

Material::Material(const std::shared_ptr<ShaderProgram>& vertex, const std::shared_ptr<ShaderProgram>& fragment) noexcept
	: vertex(vertex), fragment(fragment) {}

void Material::bind() const noexcept {
	// textures
	// for-loop to bind textures
	// uniforms (color, roughness, metallic, etc.)
}

} // namespace floyd
