#pragma once

#include <cstring>
#include <floydia/types.hpp>
#include <floydia/helpers/logger.hpp>
#include <glad/gl.h>

// Ring buffer is a fixed size memory region where the CPU writes data
// into the next avaiable slot, and after reaching the end, continues from
// the beginning.
// Slots are reused only after the GPU has finished consuming their previous contents

// Fences are GPU synchronization primitives that enforce this guarantee.
// After issuing draw calls, a fence is placed to mark that slot as in-flight.
// Before writing to a slot, the CPU waits on its fence until the GPU signals completion.
// Without fences, the ring buffer is a convention with no enforcement
// nothing stops the CPU from overwriting a slot the GPU is still reading.

namespace floyd {

// Base class for Persistent Mapped Buffers.
// Manages allocation, mapping, and CPU-side writes using a ring buffer layout
class PersistentMappedBuffer {
	public:
		// Number of frame regions (ring buffer sizze)
		static constexpr uint32_t FRAMES_IN_FLIGHT = 3;
		// Persistent Mapper buffer type
		enum BufferType : uint8 {
			SHADER_STORAGE_BUFFER,
			UNIFORM_BUFFER
		};

	public:
		// Creates a persistently mapped buffer divided into N frame regions.
		// 'binding' is the shader binding point 'layout(binding = X)'.
		// 'perframesize' is aligned internally to meet OpenGL offset requirements
		PersistentMappedBuffer(const BufferType type, const uint32 binding, const size_t perframesize) noexcept;
		~PersistentMappedBuffer() noexcept;

		inline GLuint get_id() const noexcept { return this->buffer; };
		inline size_t get_perframesize() const noexcept { return this->perframesize; };

		// Copies CPU data directly into mapped GPU memory at given offset
		inline void update(const void* data, const size_t size, const size_t offset) const noexcept {
			if(offset + size > this->capacity) {
				TRACELOG(
					logger::Error,
					"%s overflow. size=%zu, offset=%zu, capacity=%zu",
					this->enum_to_str(this->btype).c_str(),
					size, offset, this->capacity
				);
				return;
			}
			std::memcpy(static_cast<char*>(this->mapped) + offset, data, size);
		}

		// Resize buffer with new size if necessary.
		// Does not flush
		void resize(const size_t new_perframesize) noexcept;

		// Makes written data visible to GPU and binds the relevant buffer range.
		// - SSBO: Range binding + Requires memory barrier.
		// - UBO: Requires only range binding
		virtual inline void flush(const size_t offset, const size_t size) const noexcept = 0;

		// Block the CPU until the GPU has finished reading this frame's buffer slot.
		// Must be called before writing to the slot to avoid overwriting data still in use
		void wait(const uint32 frameindex) noexcept {
			GLsync& fence = this->fences[frameindex];
			if(!fence) return;
			glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
			glDeleteSync(fence);
			fence = nullptr;
		}

		// Insert a fence after draw calls are issued for this frame.
		// Signals to future wait() calls when it is safe to reuse this buffer slot
		void lock(const uint32 frameindex) noexcept {
			if(this->fences[frameindex]) glDeleteSync(this->fences[frameindex]);
			this->fences[frameindex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		}


		// Returns byte offset for the given frame's region.
		// Ensures each frame writes to a separate memory slice (avoids CPU/GPU overlap)
		inline size_t frame_offset(const uint32 frameindex) const noexcept {
			if(frameindex > FRAMES_IN_FLIGHT) {
				TRACELOG(logger::Error,
					"'frameindex' (%zu) out of ring buffer range (%d)",
					frameindex, FRAMES_IN_FLIGHT);
				return 0;
			}
			return frameindex * this->perframesize;
		}

	protected:
		GLsync fences[FRAMES_IN_FLIGHT] = {};
		void* mapped = nullptr;
		size_t perframesize = 0; // Sizze of one frame region
		size_t capacity = 0; // Total buffer size
		const uint32 binding;
		GLuint buffer = 0;
		GLint alignment = 0;
		const BufferType btype;

	private:
		inline std::string enum_to_str(const BufferType type) const noexcept { return (type == SHADER_STORAGE_BUFFER) ? "Shader Storage Buffer" : "Uniform Buffer"; }
		void make_buffer(const size_t perframesize) noexcept;
		void delete_fences() noexcept;
};

}
