#include <floydia/gfx/material.hpp>

namespace floyd {

Material::Material(const std::shared_ptr<ShaderProgram>& shader) noexcept
	: shader(shader) {}

void Material::bind() const noexcept {
	this->shader->bind();
	// for-loop to bind textures
}

} // namespace floyd
