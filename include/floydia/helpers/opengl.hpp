#pragma once

#include "floydia/types.hpp"

#include <glad/gl.h>


#define GL_CHECK_ERROR() floyd::opengl::check_gl_error(__FILE__, __LINE__)
#define GL_CHECK(x) x; { GLenum e = glGetError(); if(e) printf(#x " -> 0x%x\n", e); }

namespace floyd {
namespace opengl {
	// Returns GLenum from a type
	template <typename T>
	constexpr GLenum to_glenum();
	// Track OpenGL errors in real time
	void track_gl_error();

	// Check OpenGL error
	void check_gl_error(const char* file, const int line);
	// Extract texture format from channels
	GLuint channel_to_format(const u8 channels, const bool internal);

	template<> constexpr GLenum to_glenum<float>()  { return GL_FLOAT; }
	template<> constexpr GLenum to_glenum<f64>() { return GL_DOUBLE; }
	template<> constexpr GLenum to_glenum<u8>()  { return GL_UNSIGNED_BYTE; }
	template<> constexpr GLenum to_glenum<i8>()  { return GL_BYTE; }
	template<> constexpr GLenum to_glenum<u16>() { return GL_UNSIGNED_SHORT; }
	template<> constexpr GLenum to_glenum<i16>() { return GL_SHORT; }
	template<> constexpr GLenum to_glenum<u32>() { return GL_UNSIGNED_INT; }
	template<> constexpr GLenum to_glenum<int>() { return GL_INT; }

} // namespace opengl
} // namespace floyd
