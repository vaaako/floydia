#include <cstring>
#include <floydia/gpu/ssbo.hpp>
#include <floydia/helpers/logger.hpp>

namespace floyd {

ShaderStorageBuffer::ShaderStorageBuffer(const uint32 binding, const size_t perframesize) noexcept
	: PersistentMappedBuffer(BufferType::SHADER_STORAGE_BUFFER, binding, perframesize) {}

} // namespace floyd
