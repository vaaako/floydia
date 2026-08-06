#include "floydia/rendering/image.hpp"
#include "floydia/core/core.hpp"
#include "floydia/core/assets.hpp"
#include "floydia/helpers/logger.hpp"

#include "glad/gl.h"

#include "floydia/rgfwimpl.hpp"

namespace floyd {

size_t Core::thread_count = 0;

Core::Core() noexcept {
	RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
	hints->major = 4;
	hints->minor = 6;
	RGFW_setGlobalHints_OpenGL(hints);
}

void Core::initialize() noexcept {
	// was initialized already
	if(this->assets != nullptr
#if !defined(FLOYD_SINGLE_THREAD)
	&& this->jobsystem != nullptr
#endif
	) return;

	// Initialize GLAD
	if(!gladLoadGLLoader((GLADloadproc)RGFW_getProcAddress_OpenGL)) {
		TRACELOG(logger::Error, "Failed to initialize GLAD!");
		return;
	}

	// Initialize OpenGL dependend objects
	this->assets = std::make_unique<Assets>();
#if !defined(FLOYD_SINGLE_THREAD)
	this->jobsystem = std::make_unique<JobSystem>(Core::thread_count);
#endif
	Image::init_stb_image(); // Enable flip vertically once

	TRACELOG(logger::Info, "OpenGL initialized!");
	TRACELOG(logger::Info, "GL Version: %s", glGetString(GL_VERSION));
	TRACELOG(logger::Info, "GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	TRACELOG(logger::Info, "Vendor: %s", glGetString(GL_VENDOR));
	TRACELOG(logger::Info, "Renderer: %s", glGetString(GL_RENDERER));
}

}
