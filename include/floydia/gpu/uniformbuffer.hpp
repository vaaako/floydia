#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/opengl.hpp>

namespace floyd {

class UniformBuffer final {
	public:
		const size_t capacity;
	public:
		// 'capacity' is the 'sizeof(UBO struct)'
		UniformBuffer(const size_t capacity) noexcept;
		~UniformBuffer() noexcept;
		// Initialize Uniform Buffer.
		// 'binding' is the Uniform index inside the buffer.
		void init(const uint32 binding) noexcept;

		// Updates the Uniform Buffer data
		void update(const void* data, const size_t capacity, const size_t offset = 0) const noexcept;

	private:
		GLuint ubo;
};

} // namespace floyd
