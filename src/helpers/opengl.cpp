#include "floydia/helpers/opengl.hpp"
#include "floydia/helpers/logger.hpp"
#include <iostream>

namespace floyd {
namespace opengl {

	void track_gl_error() {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Makes the callback fire on the exact call that caused the error
		glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
			if(severity == GL_DEBUG_SEVERITY_NOTIFICATION) return; // Ignore notifications
			TRACELOG(logger::Debug, "GL ERROR: type=0x%x severity=0x%x message=%s", type, severity, message);
		}, nullptr);
	}

	GLuint channel_to_format(const u8 channels, const bool internal) {
		switch(channels) {
			case 1:
				return (internal) ? GL_R8 : GL_RED;
				break;
			case 3:
				return (internal) ? GL_RGB8 : GL_RGB;
				break;
			default:
				return (internal) ? GL_RGBA8 : GL_RGBA;
				break;
		}
	}

	void check_gl_error(const char* file, const int line) {
		GLenum error;
		while((error = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error " << error << " at " << file << ":" << line << std::endl;
		}
	}

}
} // namespace floyd
