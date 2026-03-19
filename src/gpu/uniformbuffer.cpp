#include <floydia/gpu/uniformbuffer.hpp>
#include <floydia/utilities/log.hpp>

namespace floyd {

UniformBuffer::UniformBuffer(const size_t capacity) noexcept
	: capacity(capacity) {}

void UniformBuffer::init(const uint32 binding) noexcept {
	glCreateBuffers(1, &this->ubo);
	glNamedBufferStorage(
		this->ubo,
		this->capacity,
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
	if(offset + size > this->capacity) {
		TRACELOG(log::type::Error, "Uniform Buffer overflow. size=%zu, offset=%zu, capacity=%zu", size, offset, this->capacity);
		return;
	}
	glNamedBufferSubData(this->ubo, offset, size, data);
}

} // namespace floyd
