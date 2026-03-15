#pragma once

#include <iostream>
#include <cstdint>
#include <floydia/utilities/string.hpp>

#define TRACELOG(level, ...)     floyd::log::TraceLog(level, __VA_ARGS__)
#define TRACELOG_LOC(level, ...) floyd::log::TraceLogLoc(level, __func__, __VA_ARGS__)

namespace floyd {
namespace log {
	enum class Type : uint8_t {
		Trace,
		Debug,
		Info,
		Warning,
		Error
	};

	// Returns current time as string.
	// Format: %Y-%m-%d %H:%M:%S
	std::string timestring() noexcept;

	// Converts 'log::Type' to 'const char*'
	const char* typetostr(const Type type) noexcept;

	// Trace in format:
	// '[type] message'
	template <typename... Args>
	void TraceLog(const Type type, const char* format, Args&&... args) {
		std::cout
			<< typetostr(type)
			<< " " << string::format(format, std::forward<Args>(args)...)
		<< '\n';
	}

	// Trace in format:
	// '[location][type] message'
	template <typename... Args>
	void TraceLogLoc(const Type type, const char* location, const char *format, Args &&...args) {
		std::cout
			<< "[" << location << "]"
			<< typetostr(type)
			<< " " << string::format(format, std::forward<Args>(args)...)
		<< '\n';
	}

} // namespace log
} // namespace floyd
