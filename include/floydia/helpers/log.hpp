#pragma once

#include <iostream>
#include <cstdint>
#include <floydia/helpers/string.hpp>

#define TRACELOG(level, ...)     floyd::log::TraceLog(level, __VA_ARGS__)
#define TRACELOG_LOC(level, ...) floyd::log::TraceLogLoc(level, __func__, __VA_ARGS__)

namespace floyd {
namespace log {
	enum class type : uint8_t {
		Trace,
		Debug,
		Info,
		Warning,
		Error
	};

	// Returns current time as string.
	// Format: %Y-%m-%d %H:%M:%S
	std::string timestring() noexcept;

	// Converts 'log::type' to 'const char*'
	const char* typetostr(const type t) noexcept;

	// Trace in format:
	// '[type] message'
	template <typename... Args>
	void TraceLog(const type t, const char* format, Args&&... args) {
		std::cout
			<< typetostr(t)
			<< " " << string::format(format, std::forward<Args>(args)...)
		<< '\n';
	}

	// Trace in format:
	// '[location][type] message'
	template <typename... Args>
	void TraceLogLoc(const type t, const char* location, const char *format, Args &&...args) {
		std::cout
			<< "[" << location << "]"
			<< typetostr(t)
			<< " " << string::format(format, std::forward<Args>(args)...)
		<< '\n';
	}

} // namespace log
} // namespace floyd
