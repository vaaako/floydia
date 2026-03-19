#include <floydia/core/window.hpp>

#define RGFW_IMPLEMENTATION
#define RGFW_OPENGL
#include <RGFW.h>

#include <floydia/gpu/opengl.hpp>
#include <floydia/utilities/log.hpp>
#include <iostream> // std::cerr

namespace floyd {

struct Window::impl {
	RGFW_window *window;
};

Window::Window(const Settings &settings)
	: renderer(Core::get().renderer), pimpl(std::make_unique<impl>()),
	title(settings.title), width(settings.width), height(settings.height) {
		// Initialize Window
		pimpl->window = RGFW_createWindow(
				settings.title.c_str(), 0, 0, settings.width, settings.height,
				RGFW_windowCenter | RGFW_windowOpenGL);
		if(!settings.resizable) {
			RGFW_window_setFlags(pimpl->window, RGFW_windowNoResize);
		}
		if(pimpl->window == NULL) {
			std::cerr << "Failed to create window!" << std::endl;
			return;
		}

		// Initialize OpenGL
		RGFW_glHints *hints = RGFW_getGlobalHints_OpenGL();
		hints->major = 4;
		hints->minor = 6;
		RGFW_setGlobalHints_OpenGL(hints);
		RGFW_window_makeCurrentContext_OpenGL(pimpl->window);
		if(!gladLoadGLLoader((GLADloadproc)RGFW_getProcAddress_OpenGL)) {
			std::cerr << "Failed to create window!" << std::endl;
			return;
		}
		TRACELOG(log::type::Info, "OpenGL initialized!");
		TRACELOG(log::type::Info, "GL Version: %s", glGetString(GL_VERSION));
		TRACELOG(log::type::Info, "GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
		TRACELOG(log::type::Info, "Vendor: %s", glGetString(GL_VENDOR));
		TRACELOG(log::type::Info, "Renderer: %s", glGetString(GL_RENDERER));
		// NOTE: Window should't actually initialize 'Renderer'. But I don't want user
		// to initialize it manually
		Core::get().renderer.init();

		RGFW_window_swapInterval_OpenGL(pimpl->window, RGFW_FALSE);

		TRACELOG(log::type::Info, "Window initialized completely!");
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
	// Clear previous events
	this->events.clear();
	// Mark new delta time
	this->clock.mark();
	// Poll events
	RGFW_event event;
	while(RGFW_window_checkEvent(pimpl->window, &event)) {
		this->events.emplace(static_cast<Event>(event.type));

		// Check for registered callback and run it
		if(!this->events_callbacks.empty()) {
			auto it = this->events_callbacks.find(static_cast<Event>(event.type));
			if(it != this->events_callbacks.end()) {
				it->second();
			}
		}
	}
}

void Window::swap_buffers() const noexcept {
	RGFW_window_swapBuffers_OpenGL(pimpl->window);
}

// -- SETTINGS

void Window::set_title(const std::string &title) noexcept {
	RGFW_window_setName(pimpl->window, title.c_str());
}
void Window::set_vsync(const bool state) noexcept {
	RGFW_window_swapInterval_OpenGL(pimpl->window, state);
}
void Window::set_grab_mouse(const bool state) noexcept {
	if(state) {
		RGFW_window_holdMouse(pimpl->window);
	} else {
		RGFW_window_unholdMouse(pimpl->window);
	}
}
void Window::set_hide_mouse(const bool state) noexcept {
	RGFW_window_showMouse(pimpl->window, !state);
}
void Window::set_fullscreen(const bool state) noexcept {
	RGFW_window_setFullscreen(pimpl->window, state);
}
void Window::set_border(const bool state) noexcept {
	RGFW_window_setBorder(pimpl->window, state);
}
void Window::set_opacity(const uint8 opacity) noexcept {
	RGFW_window_setOpacity(pimpl->window, opacity);
}

void Window::set_min_size(const uint32 width, const uint32 height) noexcept {
	RGFW_window_setMinSize(pimpl->window, static_cast<int>(width),
			static_cast<int>(height));
}
void Window::set_max_size(const uint32 width, const uint32 height) noexcept {
	RGFW_window_setMaxSize(pimpl->window, static_cast<int>(width),
			static_cast<int>(height));
}
void Window::viewport(const uint32 width, const uint32 height) noexcept {
	this->width = width;
	this->height = height;
	// RGFW_window_setAspectRatio(pimpl->window, static_cast<int>(width),
	// 		static_cast<int>(height));
	glViewport(0, 0, width, height);
}

bool Window::is_mouse_grabbed() const noexcept { return RGFW_window_isHoldingMouse(pimpl->window); }


vec2<uint32> Window::size() const noexcept {
	vec2<int> output;
	RGFW_window_getSize(pimpl->window, &output.x, &output.y);
	return vec2<uint32>(output);
}

// -- KEYBOARD

bool Window::keydown(const Keycode key) const noexcept {
	return RGFW_isKeyDown(static_cast<uint8>(key));
}

bool Window::keypressed(const Keycode key) const noexcept {
	return RGFW_isKeyPressed(static_cast<uint8>(key));
}

bool Window::keyup(const Keycode key) const noexcept {
	return RGFW_isKeyReleased(static_cast<uint8>(key));
}

// -- MOUSE

bool Window::mousedown(const MouseButton key) const noexcept {
	return RGFW_isMouseDown(static_cast<uint8>(key));
}

bool Window::mousepressed(const MouseButton key) const noexcept {
	return RGFW_isMousePressed(static_cast<uint8>(key));
}

bool Window::mouseup(const MouseButton key) const noexcept {
	return RGFW_isMouseReleased(static_cast<uint8>(key));
}

vec2<float> Window::mouse_scroll() const noexcept {
	vec2<float> output;
	RGFW_getMouseScroll(&output.x, &output.y);
	return output;
}

vec2<float> Window::mouse_vector() const noexcept {
	vec2<float> output;
	RGFW_getMouseVector(&output.x, &output.y);
	return output;
}

vec2<int> Window::mouse_pos() const noexcept {
	vec2<int> output;
	RGFW_window_getMouse(pimpl->window, &output.x, &output.y);
	return output;
}

// void Window::mouse_pos_global(int* x, int* y) const noexcept {
// 	RGFW_getGlobalMouse(x, y);
// }

} // namespace floyd
