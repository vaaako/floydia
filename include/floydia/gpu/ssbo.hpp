#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/opengl.hpp>

namespace floyd {

class ShaderStorageBuffer final {
	public:
		// 'size' is the 'sizeof(SSBO struct)'
		ShaderStorageBuffer(const size_t size) noexcept;
		~ShaderStorageBuffer() noexcept;
		// Initialize Shader Storage Buffer.
		// 'binding' is the Uniform index inside the buffer
		void init(const uint32 binding) noexcept;

		void update(const void* data, const size_t size, const size_t offset = 0) const noexcept;
	private:
		size_t size;
		GLuint ssbo;

};

} // namespace floyd
