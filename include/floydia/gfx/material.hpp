#pragma once

#include <floydia/gpu/programpipeline.hpp>
#include <memory>

namespace floyd {

// TODO: not finished

// Stores renderer information:
// Color, Texture and Shader
class Material {
	public:
		std::shared_ptr<ShaderProgram> vertex;
		std::shared_ptr<ShaderProgram> fragment;

	public:
		Material(const std::shared_ptr<ShaderProgram>& vertex, const std::shared_ptr<ShaderProgram>& fragment) noexcept;
		~Material() = default;

		// shared_ptr handles default
		// Copy
		Material(const Material&) = default;
		Material& operator=(const Material&) = default;
		// Move
		Material(Material&&) = default;
		Material& operator=(Material&&) = default;

	public:
		//std::vector<std::shared_ptr<Texture>>

		// Bind shader and textures.
		// Send color uniform to shader
		void bind() const noexcept;
};

} // namespace floyd
