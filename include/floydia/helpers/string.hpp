#pragma once

#include "floydia/types.hpp"
#include <stdexcept>
#include <string>
#include <vector>

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
	inline std::string format(const char* fmt) { return std::string(fmt); }
	// Find base dir of path
	inline std::string base_dir(const std::string& filepath) {
		if(filepath.find_last_of("/\\") != std::string::npos) return filepath.substr(0, filepath.find_last_of("/\\"));
		return "";
	}

	std::vector<u8> read_file_bytes(const char* path) noexcept;

} // namespace string
} // namespace floyd
