#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/vertexlayout.hpp>
#include <floydia/helpers/logger.hpp>
#include <vector>

#include <glad/gl.h>

namespace floyd {

// Geometry asset
class Mesh final {
	public:
		// Vertices size. Used for dynamic Mesh update
		const size_t capacity;
		// Size of vertices type
		const size_t vertex_type_size;
		// Indicies amount
		const size_t index_count;
		// Indices Type
		const GLenum index_type;

	public:
		template <typename T, typename U>
		Mesh(
			const std::vector<T>& vertices,
			const std::vector<U>& indices,
			const VertexLayout& layout,
			const bool dynamic = false
		);
		~Mesh() noexcept;

		inline GLuint vaoid() const noexcept { return this->vao; }
		inline GLuint vboid() const noexcept { return this->vbo; }
		inline GLuint eboid() const noexcept { return this->ebo; }

		// Allocate data once.
		// Sets fixed buffer size
		inline void allocate(const void* data, const bool dynamic) const noexcept {
			glNamedBufferStorage(
				this->vbo,
				this->vertex_type_size * this->capacity,
				data,
				(dynamic) ? GL_DYNAMIC_STORAGE_BIT : 0
			);
		}

		// Update existing data. Only works if set as 'dynamic'
		void update(const void* data, const size_t size) const noexcept;

	private:
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
		bool is_dynamic = false;
};

template <typename T, typename U>
Mesh::Mesh(const std::vector<T>& vertices, const std::vector<U>& indices, const VertexLayout& layout, const bool dynamic)
	: capacity(vertices.size()),
	vertex_type_size(sizeof(T)),
	index_count(indices.size()),
	index_type(opengl::to_glenum<U>()),
	is_dynamic(dynamic) {

	// C-like memory layout (no C++ bloat like virtual)
	static_assert(std::is_standard_layout_v<T>, "T must be standard layout (no inheritance/virtuals)");
	// Safely copied as raw bytes (no custom copy logic or hidden ownership)
	static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
	static_assert(std::is_unsigned_v<U>, "U must be an unsigned arithmetic");

	if(vertices.empty() || indices.empty()) {
		TRACELOG(logger::Error, "Vertices and Indices cannot be empty. Aborting.");
		return;
	}

	// -- VAO and VBO
	glCreateVertexArrays(1, &this->vao);
	glCreateBuffers(1, &this->vbo);
	// Allocate data on VBO
	this->allocate(vertices.data(), dynamic);

	// TODO: make indices optional later?

	// -- EBO
	glCreateBuffers(1, &this->ebo);
	glNamedBufferStorage(
		this->ebo,
		static_cast<GLsizeiptr>(indices.size() * sizeof(U)),
		indices.data(), 0
	);
	// Attach EBO to VAO
	glVertexArrayElementBuffer(this->vao, this->ebo);

	// -- Vertex Binding
	// Initialize
	glVertexArrayVertexBuffer(
		this->vao,
		0,
		this->vbo,
		0,
		layout.get_stride()
	);

	for(const VertexLayout::Attribute& attr : layout.get()) {
		glEnableVertexArrayAttrib(this->vao, attr.index);
		// Attach VBO to VAO
		glVertexArrayAttribBinding(this->vao, attr.index, 0);
		glVertexArrayAttribFormat(
			this->vao,
			attr.index,
			attr.count,
			attr.type,
			attr.normalized,
			attr.offset
		);
	}
}

} // namespace floyd
