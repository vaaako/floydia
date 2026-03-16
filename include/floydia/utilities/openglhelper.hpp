#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/opengl.hpp>


#define GL_CHECK_ERROR() floyd::openglhelper::check_gl_error(__FILE__, __LINE__)

namespace floyd {
namespace openglhelper {
	// Returns GLenum from a type
	template <typename T>
	constexpr GLenum to_glenum();

	// Check OpenGL error
	void check_gl_error(const char* file, const int line);

	template<> constexpr GLenum to_glenum<float>()  { return GL_FLOAT; }
	template<> constexpr GLenum to_glenum<double>() { return GL_DOUBLE; }
	template<> constexpr GLenum to_glenum<uint8>()  { return GL_UNSIGNED_BYTE; }
	template<> constexpr GLenum to_glenum<int8>()   { return GL_BYTE; }
	template<> constexpr GLenum to_glenum<uint16>() { return GL_UNSIGNED_SHORT; }
	template<> constexpr GLenum to_glenum<int16>()  { return GL_SHORT; }
	template<> constexpr GLenum to_glenum<uint32>() { return GL_UNSIGNED_INT; }
	template<> constexpr GLenum to_glenum<int>()    { return GL_INT; }

} // namespace opengl
} // namespace floyd
