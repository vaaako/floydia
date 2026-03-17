#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/opengl.hpp>

namespace floyd {

class UniformBuffer final {
	public:
		// `size` is the `sizeof(UBO struct)`
		UniformBuffer(const size_t size) noexcept;
		~UniformBuffer() noexcept;
		// Initialize Uniform Buffer.
		// 'binding' is the Uniform index inside the buffer.
		void init(const uint32 binding) noexcept;

		void update(const void* data, const size_t size, const size_t offset = 0) const noexcept;

	private:
		size_t size;
		GLuint ubo;
		// GLuint binding;
};

} // namespace floyd
