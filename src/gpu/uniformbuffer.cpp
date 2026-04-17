#include <floydia/gpu/uniformbuffer.hpp>
#include <floydia/helpers/log.hpp>

namespace floyd {

UniformBuffer::UniformBuffer(const uint32 binding, const size_t perframesize) noexcept
	: PersistentMappedBuffer(BufferType::UNIFORM_BUFFER, binding, perframesize) {}

} // namespace floyd
