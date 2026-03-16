#include <floydia/utilities/hash.hpp>

#include <cstdint>

namespace floyd {

size_t hash::fnv1a(const void* data, const size_t size) noexcept {
	const uint8_t* bytes = static_cast<const uint8_t*>(data);
	size_t hash = 14695981039346656037ull; // 64-bit offset basis
	for(size_t i = 0; i < size; i++) {
		hash ^= bytes[i];
		hash *= 1099511628211ull; // FNV prime
	}
	return hash;
}

} // namespace floyd
