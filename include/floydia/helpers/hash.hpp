#pragma once

#include <cstddef>
#include <functional>
#include <string>

namespace floyd {
namespace hash {
	// From boost library.
	// Makes a hash out of a value and returns it.
	// WARNING: Do NOT use raw pointers (like 'char*')
	template <typename T>
	size_t make(const T& value) noexcept {
		size_t seed = 0;
		seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}

	// Hash any value
	// WARNING: Do NOT use raw pointers (like 'char*')
	template <typename T>
	void combine(size_t& seed, const T& value) noexcept { seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2); }

	template <typename T>
	inline size_t of(const T& value) noexcept {
		size_t seed = 0;
		hash::combine(seed, value);
		return seed;
	}
	inline size_t of(const char* s) noexcept            { return hash::of<std::string_view>(std::string_view(s)); }
	inline size_t of(const std::string& s) noexcept     { return hash::of<std::string_view>(s); }
	inline size_t of(const std::string_view s) noexcept { return hash::of<std::string_view>(s); }

	size_t of(const void* data, const size_t size) noexcept;

	// Fast, deterministic, non-cryptographic hash for raw bytes.
	// Use this for asset caching (textures, shaders, files).
	// WARNING: Do NOT use raw pointers (like 'char*')
	size_t fnv1a(const void* data, const size_t size) noexcept;

} // namespace hash
} // namespace floyd
