#pragma once

#include <stdexcept>
#include <string>

namespace floyd {
namespace string {
	// C style format
	template <typename... Args>
	std::string format(const char* fmt, Args&&...args) {
		// Determine required size
		int size = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
		if(size < 0) {
			throw std::runtime_error("Error during formatting");
		}

		// Create buffer and format
		std::string result;
		result.resize(size);
		std::snprintf(result.data(), size + 1, fmt, std::forward<Args>(args)...);
		return result;
	}

	// Just a fallback in case 'Args...' is empty
	inline std::string format(const char* fmt) {
		return std::string(fmt);
	}

} // namespace string
} // namespace floyd
