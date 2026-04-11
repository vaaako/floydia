#include <floydia/gpu/ssbo.hpp>
#include <floydia/helpers/log.hpp>

namespace floyd {

ShaderStorageBuffer::ShaderStorageBuffer(const uint32 binding, const size_t capacity) noexcept
	: capacity(capacity) {

	glCreateBuffers(1, &this->ssbo);
	glNamedBufferStorage(
		this->ssbo,
		this->capacity,
		NULL,
		GL_DYNAMIC_STORAGE_BIT
	);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, this->ssbo);
}

ShaderStorageBuffer::~ShaderStorageBuffer() noexcept {
	// glUnmapNamedBuffer(this->ssbo);
	glDeleteBuffers(1, &this->ssbo);
}

void ShaderStorageBuffer::update(const void* data, const size_t size, const size_t offset) const noexcept {
	if(offset + size > this->capacity) {
		TRACELOG(log::type::Error, "Shader Storage Buffer overflow. size=%zu, offset=%zu, capacity=%zu", size, offset, this->capacity);
		return;
	}
	glNamedBufferSubData(this->ssbo, offset, size, data);
}

} // namespace floyd
