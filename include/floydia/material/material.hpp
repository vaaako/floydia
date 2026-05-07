#pragma once

#include <floydia/material/texture.hpp>
#include <floydia/gpu/programpipeline.hpp>
#include <memory>

namespace floyd {

// Shaders
struct Material {
	std::shared_ptr<ShaderProgram> vertex;
	std::shared_ptr<ShaderProgram> fragment;

	inline Material(const std::shared_ptr<ShaderProgram>& vertex, const std::shared_ptr<ShaderProgram>& fragment) noexcept
		: vertex(vertex), fragment(fragment) {}
	~Material() = default;
};

} // namespace floyd
