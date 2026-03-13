#include <floydia/utilities/openglhelper.hpp>
#include <iostream>

namespace floyd {

	void openglhelper::check_gl_error(const char* file, const int line) {
		GLenum error;
		while((error = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error " << error << " at " << file << ":" << line << std::endl;
		}
	}

} // namespace floyd
