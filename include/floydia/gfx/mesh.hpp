#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/opengl.hpp>
#include <floydia/gpu/vertexlayout.hpp>
#include <vector>


namespace floyd {

// Geometry asset
class Mesh final {
	public:
		// Indicies amount
		const size_t index_count;
		// Size of vertices type
		const uint32 vert_size;
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


		inline void upload_data(const void* data, const size_t capacity, const bool dynamic) const noexcept {
			glNamedBufferData(
				this->vbo,
				this->vert_size * capacity,
				data,
				(dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW
			);
		}

	private:
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
};

template <typename T, typename U>
Mesh::Mesh(const std::vector<T>& vertices, const std::vector<U>& indices, const VertexLayout& layout, const bool dynamic)
	: index_count(indices.size()), vert_size(sizeof(T)), index_type(openglhelper::to_glenum<U>()) {

	static_assert(std::is_standard_layout_v<T>);
	// static_assert(std::is_base_of_v<Vertex, T>, "T must derive from Vertex");
	static_assert(std::is_unsigned_v<U>, "U must be an unsigned arithmetic");

	// -- VAO and VBO
	glCreateVertexArrays(1, &this->vao);
	glCreateBuffers(1, &this->vbo);
	// Allocate data on VBO
	this->upload_data(vertices.data(), vertices.size(), dynamic);

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
		glVertexArrayAttribFormat(
			this->vao,
			attr.index,
			attr.count,
			attr.type,
			attr.normalized,
			attr.offset
		);
		// Attach VBO to VAO
		glVertexArrayAttribBinding(this->vao, attr.index, 0);
	}
}

} // namespace floyd
