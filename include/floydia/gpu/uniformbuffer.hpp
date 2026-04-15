#pragma once

#include <floydia/types.hpp>

#include <glad/gl.h>

namespace floyd {

class UniformBuffer final {
	public:
		const size_t capacity;
	public:
		// 'binding' is the Uniform index inside the buffer.
		// 'capacity' is the 'sizeof(UBO struct)'
		UniformBuffer(const uint32 binding, const size_t capacity) noexcept;
		~UniformBuffer() noexcept;

		// Updates the Uniform Buffer data
		void update(const void* data, const size_t capacity, const size_t offset = 0) const noexcept;

	private:
		GLuint ubo;
};

} // namespace floyd
