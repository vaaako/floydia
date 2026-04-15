#include <floydia/core/core.hpp>
#include <floydia/helpers/log.hpp>

#include <floydia/libsimpl.hpp>
#include <glad/gl.h>

namespace floyd {

Core::Core() noexcept {
	RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
	hints->major = 4;
	hints->minor = 6;
	RGFW_setGlobalHints_OpenGL(hints);
	this->instance = this;
}

void Core::initialize() noexcept {
	if(this->initialized) {
		return; // was initialized already
	}

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)RGFW_getProcAddress_OpenGL)) {
		std::cerr << "Failed to create window!" << std::endl;
		return;
	}

	// Initialize OpenGL dependend objects
	this->assets = std::make_unique<Assets>(this->buffermanager);
	this->renderer = std::make_unique<Renderer>();
	this->initialized = true;

	TRACELOG(log::type::Info, "OpenGL initialized!");
	TRACELOG(log::type::Info, "GL Version: %s", glGetString(GL_VERSION));
	TRACELOG(log::type::Info, "GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	TRACELOG(log::type::Info, "Vendor: %s", glGetString(GL_VENDOR));
	TRACELOG(log::type::Info, "Renderer: %s", glGetString(GL_RENDERER));
}

}
