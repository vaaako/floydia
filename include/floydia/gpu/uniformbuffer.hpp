#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/opengl.hpp>

namespace floyd {

class UniformBuffer final {
	public:
		// - `binding` is the Uniform index inside the buffer.
		// - `size` is the `sizeof(UBO struct)`. std140-aligned
		UniformBuffer(const uint32 binding, const size_t size) noexcept;
		~UniformBuffer() noexcept;

		void update(const void* data, const size_t size, const size_t offset = 0) const noexcept;

	private:
		size_t size;
		GLuint ubo;
		// GLuint binding;
};

} // namespace floyd
