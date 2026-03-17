#include <floydia/core/window.hpp>

#define RGFW_IMPLEMENTATION
#define RGFW_OPENGL
#include <RGFW.h>

#include <iostream> // std::cerr
#include <floydia/gpu/opengl.hpp>
#include <floydia/utilities/log.hpp>

namespace floyd {

struct Window::impl {
	RGFW_window* window;
};

Window::Window(const Settings& settings)
	: renderer(Core::get().renderer),
	pimpl(std::make_unique<impl>()),
	width(settings.width), height(settings.height), title(settings.title) {
	// Initialize Window
	pimpl->window = RGFW_createWindow(
		settings.title.c_str(),
		0, 0,
		settings.width, settings.height,
		RGFW_windowCenter |
		RGFW_windowNoResize |
		RGFW_windowOpenGL
	);
	if(pimpl->window == NULL) {
		std::cerr << "Failed to create window!" << std::endl;
		return;
	}

	// Initialize OpenGL
	RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
	hints->major = 4;
	hints->minor = 5;
	RGFW_setGlobalHints_OpenGL(hints);
	RGFW_window_makeCurrentContext_OpenGL(pimpl->window);
	if(!gladLoadGLLoader((GLADloadproc)RGFW_getProcAddress_OpenGL)) {
		std::cerr << "Failed to create window!" << std::endl;
		return;
	}
	TRACELOG(log::type::Trace, "OpenGL 4.5 initialized!");
	// NOTE: Window should't actually initialize 'Renderer'. But I don't want user to initialize it manually
	Core::get().renderer.init();

	RGFW_window_swapInterval_OpenGL(pimpl->window, settings.vsync);

	TRACELOG(log::type::Trace, "Window initialized completely!");
}

Window::~Window() {
	if(pimpl->window != nullptr) {
		RGFW_window_close(pimpl->window);
	}
}

bool Window::is_open() const noexcept {
	return RGFW_window_shouldClose(pimpl->window) == RGFW_FALSE;
}

void Window::poll_events() noexcept {
	RGFW_event event;
	while(RGFW_window_checkEvent(pimpl->window, &event)) {
		this->events.emplace(static_cast<Event>(event.type));
	}
}

void Window::swap_buffers() const noexcept {
	RGFW_window_swapBuffers_OpenGL(pimpl->window);
}

} // namespace floyd
