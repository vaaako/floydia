#include "floydia/rendering/image.hpp"
#include "floydia/core/core.hpp"
#include "floydia/core/assets.hpp"
#include "floydia/helpers/logger.hpp"

#include "glad/gl.h"

#include "floydia/rgfwimpl.hpp"

namespace floyd {

Core::Core() noexcept {
	RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
	hints->major = 4;
	hints->minor = 6;
	RGFW_setGlobalHints_OpenGL(hints);
}

void Core::initialize() noexcept {
	// was initialized already
	if(this->assets != nullptr && this->renderer != nullptr) return;

	// Initialize GLAD
	if(!gladLoadGLLoader((GLADloadproc)RGFW_getProcAddress_OpenGL)) {
		TRACELOG(logger::Error, "Failed to initialize GLAD!");
		return;
	}

	// Initialize OpenGL dependend objects
	this->assets = std::make_unique<Assets>();
	this->renderer = std::make_unique<Renderer>();

	// Enable flip vertically once
	Image::init_stb_image();

	TRACELOG(logger::Info, "OpenGL initialized!");
	TRACELOG(logger::Info, "GL Version: %s", glGetString(GL_VERSION));
	TRACELOG(logger::Info, "GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	TRACELOG(logger::Info, "Vendor: %s", glGetString(GL_VENDOR));
	TRACELOG(logger::Info, "Renderer: %s", glGetString(GL_RENDERER));
}

}
