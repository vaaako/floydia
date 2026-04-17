#include <floydia/gfx/mesh.hpp>

namespace floyd {

Mesh::~Mesh() noexcept {
	glDeleteBuffers(1, &this->ebo);
	glDeleteBuffers(1, &this->vbo);
	glDeleteVertexArrays(1, &this->vao);
}

void Mesh::update(const void* data, const size_t size) const noexcept {
	if(!this->is_dynamic) {
		TRACELOG(logger::Error, "Buffer is not dynamic. Aborting.");
		return;
	}

	if(size > this->capacity) {
		TRACELOG(logger::Error, "Buffer overflow. Aborting.");
		return;
	}

	glNamedBufferSubData(
		this->vbo,
		0,
		this->vertex_type_size * size,
		data
	);
}

} // namespace floyd
