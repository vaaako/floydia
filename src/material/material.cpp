#include "floydia/material/material.hpp"
#include "floydia/core/core.hpp"
#include "floydia/helpers/hash.hpp"

namespace floyd {

Material::Material(const std::shared_ptr<ShaderProgram>& vertex, const std::shared_ptr<ShaderProgram>& fragment) noexcept
	: vertex(vertex), fragment(fragment) {
	this->albedo = assets().load<Texture>(hash::of("d_white"));
}

void Material::bind() const noexcept {
	this->albedo->bind(0);
	if(this->on_bind) this->on_bind();
	// NOTE: for shaders that don't have these uniforms, it will simply ignore
}

} // namespace floyd
