#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/persistentmappedbuffer.hpp>

#include <glad/gl.h>

namespace floyd {

class UniformBuffer final : public PersistentMappedBuffer {
	public:
		// Creates a persistently mapped buffer divided into N frame regions.
		// 'binding' is the shader binding point 'layout(binding = X)'.
		// 'perframesize' is aligned internally to meet OpenGL offset requirements
		UniformBuffer(const u32 binding, const size_t capacity) noexcept;
		~UniformBuffer() = default;

		// Makes written data visible to GPU and binds the relevant buffer range.
		inline void flush(const size_t offset, const size_t size) const noexcept override {
			// flush only what was written
			glFlushMappedNamedBufferRange(this->buffer, offset, size);
			// Defines which slice of the buffer the shader will read
			glBindBufferRange(
				GL_UNIFORM_BUFFER,
				this->binding, // 'layout(binding = x)' in shader
				this->buffer,
				offset, // Start of this frame's data
				size // How much is visible
			);
		}
};

} // namespace floyd
