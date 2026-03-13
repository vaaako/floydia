#include <floydia/gfx/mesh.hpp>

namespace floyd {
	Mesh::~Mesh() noexcept {
		glDeleteBuffers(1, &this->ebo);
		glDeleteBuffers(1, &this->vbo);
		glDeleteVertexArrays(1, &this->vao);
	}

} // namespace floyd
