#include <floydia/utilities/log.hpp>

#include <sstream>
#include <chrono>
#include <iomanip>

namespace floyd {

std::string log::timestring() noexcept {
	auto now = std::chrono::system_clock::now();
	auto time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
	return ss.str();
}

const char* log::typetostr(const log::Type type) noexcept {
	switch (type) {
		case log::Type::Trace:
			return "[TRACE]";
		case log::Type::Debug:
			return "[DEBUG]";
		case log::Type::Info:
			return "[INFO]";
		case log::Type::Warning:
			return "[WARNING]";
		case log::Type::Error:
			return "[ERROR]";
		default:
			return "[TRACE]";
	}
}

} // namespace floyd
