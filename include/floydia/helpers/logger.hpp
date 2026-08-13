#pragma once

#include <cstdint>
#include "floydia/helpers/string.hpp"

#define TRACELOG(level, ...) floyd::logger::log(level, __VA_ARGS__)

namespace floyd {
namespace logger {

enum level : uint8_t { Trace, Debug, Info, Warning, Error };

// Converts 'logger' to 'const char*'
inline const char *typetostr(const logger::level t) noexcept {
	switch (t) {
		case logger::level::Trace:
			return "[TRACE]";
		case logger::level::Debug:
			return "[DEBUG]";
		case logger::level::Info:
			return "[INFO]";
		case logger::level::Warning:
			return "[WARNING]";
		case logger::level::Error:
			return "[ERROR]";
		default:
			return "[TRACE]";
	}
}

// Trace in format:
// '[level] message'
template <typename... Args>
void log(const logger::level lvl, const char *fmt, Args &&...args) {
	std::string msg = string::format(fmt, std::forward<Args>(args)...);
	std::fprintf(stderr, "%s %s\n", typetostr(lvl), msg.c_str());
}

} // namespace logger
} // namespace floyd
