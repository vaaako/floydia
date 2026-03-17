#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/opengl.hpp>

namespace floyd {

class ShaderStorageBuffer final {
	public:
		// Shader Storage Buffer capacity
		const size_t capacity;

	public:
		// 'capacity' is the 'sizeof(SSBO struct)'
		ShaderStorageBuffer(const size_t capacity) noexcept;
		~ShaderStorageBuffer() noexcept;
		// Initialize Shader Storage Buffer.
		// 'binding' is the Uniform index inside the buffer
		void init(const uint32 binding) noexcept;

		// Updates the Shader Storage Buffer data
		void update(const void* data, const size_t capacity, const size_t offset = 0) const noexcept;
	private:
		GLuint ssbo;

};

} // namespace floyd
