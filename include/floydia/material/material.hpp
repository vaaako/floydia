#pragma once

#include "floydia/gpu/shaderprogram.hpp"
#include "floydia/material/texture.hpp"
#include <functional>
#include <memory>

namespace floyd {

// Store shaders, texture, metallic, roughness
class Material {
	public:
		std::shared_ptr<ShaderProgram> vertex;
		std::shared_ptr<ShaderProgram> fragment;
		std::shared_ptr<Texture> albedo;
		// Metallic controls specular color.
		// Metal (1.0) reflects object color, plastic (0.0) reflects white
		float metallic = 0.0f;
		// Roughness controlls shininess.
		// Low roughness = sharp highlight
		float roughness = 1.0f;
		// Called at end of bind(). Usually used to set uniforms
		std::function<void()> on_bind;

	public:
		Material(const std::shared_ptr<ShaderProgram>& vertex, const std::shared_ptr<ShaderProgram>& fragment) noexcept;
		~Material() = default;

		inline Texture* texture(const std::shared_ptr<Texture>& albedo) noexcept { return this->albedo.get(); }
		inline const Texture* texture(const std::shared_ptr<Texture>& albedo) const noexcept { return this->albedo.get(); }
		// Bind texture and call 'on_bind'
		void bind() const noexcept;
};

} // namespace floyd
