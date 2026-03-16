#pragma once

#include <floydia/gpu/shaderprogram.hpp>

namespace floyd {

class Material {
	public:
		// TODO: not finished
		Material(const std::shared_ptr<ShaderProgram>& shader) : shader(shader) {}

		std::shared_ptr<ShaderProgram> shader;
		glm::vec4 color = { 1, 1, 1, 1 };
		//std::vector<std::shared_ptr<Texture>>
		//Color color

		// TODO: not finished
		// Bind shader and texture
		inline void bind() const noexcept {
			this->shader->bind();
			this->shader->set_uniform_vec4f("u_color", color);
		}
};

} // namespace floyd
