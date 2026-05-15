#pragma once

#include "floydia/gpu/persistentmappedbuffer.hpp"
#include <floydia/types.hpp>

#include <glad/gl.h>

namespace floyd {

class ShaderStorageBuffer final : public PersistentMappedBuffer {
	public:
		// Creates a persistently mapped buffer divided into N frame regions.
		// 'binding' is the shader binding point 'layout(binding = X)'.
		// 'perframesize' is aligned internally to meet OpenGL offset requirements
		ShaderStorageBuffer(const uint32 binding, const size_t perframesize) noexcept;
		~ShaderStorageBuffer() = default;

		// Binds a chunk of the SSBO
		inline void bind(const uint32 offset, const size_t bytes) const noexcept {
			glBindBufferRange(GL_SHADER_STORAGE_BUFFER, this->binding, this->buffer, offset, bytes);
		}

		// Makes written data visible to GPU and binds the relevant buffer range.
		inline void flush(const size_t offset, const size_t size) const noexcept override {
			// flush only what was written
			glFlushMappedNamedBufferRange(this->buffer, offset, size);
			// Ensure GPU sees CPU writes before using SSBO in shaders
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			// Defines which slice of the buffer the shader will read
			glBindBufferRange(
				GL_SHADER_STORAGE_BUFFER,
				this->binding, // 'layout(binding = x)' in shader
				this->buffer,
				offset, // Start of this frame's data
				size // How much is visible
			);
		}

};

} // namespace floyd
