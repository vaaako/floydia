#pragma once

namespace floyd {
namespace hash {
	// Froom boost library.
	// Makes a hash out of a value and return it.
	// WARNING: Do NOT use raw pointers (like 'char*')
	template <typename T>
	size_t make(const T& value) noexcept {
		size_t hash = 0;
		std::hash<T> h;
		// Golden ratio
		hash ^= h(value) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		return hash;
	}

	// Froom boost library.
	// To multiple values, keep passing the same variable as 'seed' until you done.
	// WARNING: Do NOT use raw pointers (like 'char*')
	template <typename T>
	void combine(size_t& seed, const T& value) noexcept {
		std::hash<T> h;
		seed ^= h(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	// Fast, deterministic, non-cryptographic hash for raw bytes.
	// Use this for asset caching (textures, shaders, files).
	// WARNING: Do NOT use raw pointers (like 'char*')
	size_t fnv1a(const void* data, const size_t size) noexcept {
		const uint8_t* bytes = static_cast<const uint8_t*>(data);
		size_t hash = 14695981039346656037ull; // 64-bit offset basis
		for(size_t i = 0; i < size; i++) {
			hash ^= bytes[i];
			hash *= 1099511628211ull; // FNV prime
		}
		return hash;
	}

} // namespace hash
} // namespace floyd
