#include <floydia/helpers/opengl.hpp>
#include <iostream>

namespace floyd {
namespace opengl {
	GLuint channel_to_format(const uint8 channels, const bool internal) {
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
