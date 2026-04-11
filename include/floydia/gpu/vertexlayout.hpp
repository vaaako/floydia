#pragma once

#include <vector>
#include <floydia/types.hpp>
#include <floydia/helpers/opengl.hpp>

namespace floyd {

class VertexLayout final {
	public:
		struct Attribute {
			// The offset of this attribute within a vertex
			size_t offset;
			// The index of the vertex attribute in the shader
			uint32 index;
			// The number of components per attribute (e.g., 3 for a vec3)
			uint32 count;
			// Type of the indices
			GLenum type;
			// Wheter to normalize the data
			bool normalized = false;
		};

		// Automatically deals with offset and other values when adding a new attribute
		template <typename T>
		void add(const uint32 count, const bool normalized = false) noexcept;

		const std::vector<Attribute>& get() const noexcept {
			return this->attributes;
		}

		inline size_t get_stride() const noexcept {
			return this->stride;
		}

	private:
		std::vector<Attribute> attributes;
		uint32 index = 0;
		uint32 stride = 0;
};

template <typename T>
void VertexLayout::add(const uint32 count, const bool normalized) noexcept {
	static_assert(std::is_arithmetic_v<T>, "Type must be arithmetic");

	this->attributes.push_back({
		.offset = this->stride,
		.index = this->index++,
		.count = count,
		.type = opengl::to_glenum<T>(),
		.normalized = normalized
	});
	this->stride += count * sizeof(T);
}

} // namespace floyd
