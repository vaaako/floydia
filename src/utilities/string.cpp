#include <floydia/utilities/string.hpp>

#include <cstdarg>
#include <stdexcept>

namespace floyd {

std::string string::format(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	// Determine the required buffer size
	va_list args_copy;
	va_copy(args_copy, args);
	int size = std::vsnprintf(nullptr, 0, fmt, args_copy);
	va_end(args_copy);

	if(size < 0) {
		va_end(args);
		throw std::runtime_error("Error during formatting");
	}

	// Create buffer and format
	std::string result;
	result.resize(size);
	// Write formatted string to buffer
	std::vsnprintf(&result[0], size + 1, fmt, args);
	va_end(args);

	return result;
}

} // namespace floyd
