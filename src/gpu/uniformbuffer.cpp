#include <floydia/gpu/uniformbuffer.hpp>
#include <floydia/utilities/log.hpp>

namespace floyd {

UniformBuffer::UniformBuffer(const uint32 binding, const size_t size) noexcept
	: size(size) {
	glCreateBuffers(1, &this->ubo);
	glNamedBufferStorage(
		this->ubo,
		size,
		NULL,
		GL_DYNAMIC_STORAGE_BIT
	);
	// Flags: GL_DYNAMIC_STORAGE_BIT, GL_MAP_WRITE_BIT, GL_MAP_READ_BIT, GL_MAP_PERSISTENT_BIT
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, this->ubo);
}

UniformBuffer::~UniformBuffer() noexcept {
	// glUnmapNamedBuffer(this->ubo);
	glDeleteBuffers(1, &this->ubo);
}

void UniformBuffer::update(const void* data, const size_t size, const size_t offset) const noexcept {
	if(offset + size > this->size) {
		TRACELOG(log::type::Error, "Uniform Buffer overflow. Aborting.");
		return;
	}
	glNamedBufferSubData(this->ubo, offset, size, data);
}

} // namespace floyd
