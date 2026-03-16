#pragma once

#include <floydia/gpu/shaderprogram.hpp>
#include <floydia/core/assets.hpp>

namespace floyd {

// TODO: not finished

// Stores renderer information:
// Color, Texture and Shader
class Material {
	public:
		Material() = default;
		~Material() = default;

		// shared_ptr handles default
		// Copy
		Material(const Material&) = default;
		Material& operator=(const Material&) = default;
		// Move
		Material(Material&&) = default;
		Material& operator=(Material&&) = default;

	public:
		std::shared_ptr<ShaderProgram> shader = Assets::default_program();
		//std::vector<std::shared_ptr<Texture>>
		//Color color

		// Bind shader and textures.
		// Send color uniform to shader
		inline void bind() const noexcept {
			this->shader->bind();
			this->shader->set_uniform_vec4f("u_color", color);
			// for-loop to bind textures
		}

		// Set a color in a 0-255 range
		inline void set_color(const vec4<uint8> color) {
			this->color = (glm::vec4)color / glm::vec4(255.0f);
		}

	private:
		vec4<float> color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

} // namespace floyd
