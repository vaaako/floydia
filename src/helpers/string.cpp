#include "floydia/helpers/string.hpp"
#include <filesystem>
#include <fstream>

namespace floyd {
namespace string {
	std::vector<u8> read_file_bytes(const char* path) noexcept {
		if(path == nullptr) return {};
		if(!std::filesystem::exists(path)) return {};

		std::ifstream file = std::ifstream(path, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg(); // File size
		file.seekg(0, std::ios::beg);
		// Read file into buffer
		std::vector<u8> buffer = std::vector<u8>(static_cast<size_t>(size));
		if(!file.read(reinterpret_cast<char*>(buffer.data()), size)) return {};
		return buffer;
	}
}
}
