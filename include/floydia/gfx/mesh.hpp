#pragma once

#include <floydia/types.hpp>
#include <vector>


namespace floyd {

// Geometry asset
class Mesh final {
	public:
		Mesh(const std::vector<float>& vertices, std::vector<uint32> indices);

	private:
		//GLuint vao;
		//GLuint vbo;
		//GLuint ebo;

		// AABB localbounds;
};

} // namespace floyd
