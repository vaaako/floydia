#include <floydia/gpu/shader.hpp>
#include <floydia/gpu/persistentmappedbuffer.hpp>


namespace floyd {

PersistentMappedBuffer::PersistentMappedBuffer(const BufferType type, const u32 binding, const size_t perframesize) noexcept
	: binding(binding), btype(type) {
	// Query required aligment for buffers offsets
	glGetIntegerv(
		(this->btype == SHADER_STORAGE_BUFFER)
			? GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT
			: GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
		&this->alignment
	);
	if(this->alignment <= 0) this->alignment = 1; // Rare but safe
	this->make_buffer(perframesize);
}

// Unmap and delete
PersistentMappedBuffer::~PersistentMappedBuffer() noexcept {
	this->delete_fences();
	if(this->buffer != 0) {
		glUnmapNamedBuffer(this->buffer);
		this->mapped = nullptr;
		glDeleteBuffers(1, &this->buffer);
		this->buffer = 0;
	}
}

void PersistentMappedBuffer::resize(const size_t new_perframesize) noexcept {
#if defined(FLOYD_DEBUG_MAPPED_BUFFER)
	TRACELOG(logger::Info, "Resizing %s. %zu -> %zu",
		this->enum_to_str(this->btype).c_str(), this->_perframesize, new_perframesize);
#endif
	this->make_buffer(new_perframesize);
}

void PersistentMappedBuffer::make_buffer(const size_t perframesize) noexcept {
	// Align perframe region size, so each frame starts at a valid GPU boundary
	// Assert driver returned a power-of-two, then use division fallback just in case
	size_t perframe = ((perframesize + this->alignment - 1) / this->alignment) * this->alignment;
	// Total buffer = N frames (ring buffer)
	size_t capacity = perframe * FRAMES_IN_FLIGHT;

	GLuint buffer = 0;
	glCreateBuffers(1, &buffer);
	glNamedBufferStorage(
		buffer,
		capacity,
		nullptr,
		GL_MAP_WRITE_BIT | // Write from CPU
		GL_MAP_PERSISTENT_BIT // Mapping once and keeping pointer
	);

	void* mapped = glMapNamedBufferRange(
		buffer,
		0, // Offset
		capacity, // Length
		GL_MAP_WRITE_BIT |
		GL_MAP_PERSISTENT_BIT |
		GL_MAP_FLUSH_EXPLICIT_BIT // Manually tell exactly which range it was wrote
		// GL_MAP_COHERENT_BIT // Assume any byte in this mapped range may have been written at any time 
	);

	if(mapped == nullptr) {
		TRACELOG(logger::Error, "Failed to map/remap %s", this->enum_to_str(this->btype).c_str());
		glDeleteBuffers(1, &buffer); // Delete new buffer
		return;
	}

	// If has a buffer. Replace
	if(this->buffer != 0) {
		// Preserve existing data. Each ring-buffer frame slot moved to a new
		// offset, so copy slot by slot instead of a single flat copy
		const size_t copy_per_frame = std::min(this->_perframesize, perframe);
		for(u32 i = 0; i < FRAMES_IN_FLIGHT; ++i) {
			glCopyNamedBufferSubData(
				this->buffer, buffer,
				i * this->_perframesize, // Old offset
				i * perframe, // New offset
				copy_per_frame
			);
		}

		// Wait on ALL fences before deleting old buffer
		this->delete_fences();
		glUnmapNamedBuffer(this->buffer);
		glDeleteBuffers(1, &this->buffer); // Delete old buffer
		this->mapped = nullptr;
	}

	// Assign new
	this->buffer = buffer;
	this->mapped = mapped;
	this->_perframesize = perframe;
	this->capacity = capacity;
}


void PersistentMappedBuffer::delete_fences() noexcept {
	// Wait and delete all pending fences first
	for(u32 i = 0; i < FRAMES_IN_FLIGHT; i++) {
		if(this->fences[i]) {
			GLsync& fence = this->fences[i];
			glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
			glDeleteSync(fence);
			fence = nullptr;
		}
	}
}

} // namespace floyd
