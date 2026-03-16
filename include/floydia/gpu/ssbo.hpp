#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/opengl.hpp>

namespace floyd {

class ShaderStorageBuffer final {
	public:
		ShaderStorageBuffer(const uint32 binding, const size_t size) noexcept;
		~ShaderStorageBuffer() noexcept;

		void update(const void* data, const size_t size, const size_t offset = 0) const noexcept;
	private:
		size_t size;
		GLuint ssbo;

};

} // namespace floyd
