#pragma once

#include <floydia/gpu/shaderprogram.hpp>

namespace floyd {

class Material {
	public:
		std::shared_ptr<ShaderProgram> shader;
		//std::vector<std::shared_ptr<Texture>>
		//Color color

		// Bind shader and texture
		//void bind();
};

} // namespace floyd
