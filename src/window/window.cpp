#include "floydia/helpers/logger.hpp"
#include "floydia/window/window.hpp"
#include "floydia/core/core.hpp"

#include <mutex>

#include "floydia/rgfwimpl.hpp"

#if !defined(FLOYD_RELEASE)
#include "imgui/imgui_impl_opengl3.h"
#endif

// https://github.com/ColleagueRiley/RGFW/blob/main/examples/multi-window/multi-window.c

namespace floyd {

// Shared between Windows
namespace {
	std::mutex wmutex;
	RGFW_glContext* shared_context = nullptr;
	u32 wincount = 0;
}

struct Window::impl {
	RGFW_window* window;
};

Window::Window(const Settings& settings)
	: pimpl(std::make_unique<impl>()),
	title(settings.title), _width(settings.width), _height(settings.height) {

	std::lock_guard<std::mutex> lock(wmutex);
	const bool isfirst = (wincount == 0);

	// Not first window. Share context from first window
	if(!isfirst) {
		RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
		hints->share = shared_context;
		RGFW_setGlobalHints_OpenGL(hints);
	}

	// Initialize Window
	pimpl->window = RGFW_createWindow(
			settings.title.c_str(), 0, 0, settings.width, settings.height,
			RGFW_windowCenter | RGFW_windowOpenGL);

	if(pimpl->window == NULL) {
		TRACELOG(logger::Error, "Failed to create window!");
		return;
	}

	if(!settings.resizable) {
		RGFW_window_setFlags(pimpl->window, RGFW_windowNoResize);
	}

	// First window
	if(isfirst) {
		// Store shared context
		shared_context = RGFW_window_getContext_OpenGL(pimpl->window);
		this->enable_ctx(); // Enable context for this window
		// Initialize GLAD + Core
		Core::get().initialize();

	#if !defined(FLOYD_RELEASE)
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		// Initialize backend for this application
		ImGuiIO& io = ImGui::GetIO();
		io.BackendPlatformName = "floyd_rgfw";
		// Initialize OpenGL backend
		ImGui_ImplOpenGL3_Init("#version 460");
		TRACELOG(logger::Info, "ImGui initialized!");
	#endif
	}

	TRACELOG(logger::Info, "Window initialized! (%d total)", ++wincount);

	// Release context (if not first window). User must manually set current context
	if(wincount > 1) this->disable_ctx();
	this->renderer = Core::get().renderer.get();

	// Disable VSync by default
	RGFW_window_swapInterval_OpenGL(pimpl->window, RGFW_FALSE);
}

Window::~Window() {
	std::lock_guard<std::mutex> lock(wmutex);

	if(pimpl->window != nullptr) {
		TRACELOG(logger::Info, "Closing window: %p (%d remaining)", (void*)pimpl->window, --wincount);
		if(wincount == 0) {
			TRACELOG(logger::Info, "Closing OpenGL context");
			this->enable_ctx();
			this->disable_ctx();
			shared_context = nullptr;
			// Delete ImGui
			TRACELOG(logger::Info, "Closing ImGui");
			ImGui_ImplOpenGL3_Shutdown();
			// ImGui_ImplRgfw_Shutdown();
			ImGui::DestroyContext();
		}
		RGFW_window_close(pimpl->window); // Automatically calls RGFW_window_deleteContext_OpenGL
		pimpl->window = nullptr;
	}
}


vec2<u32> sttc_size() noexcept {
	int w, h;
	RGFW_window_getSize(RGFW_getCurrentWindow_OpenGL(), &w, &h);
	return { w, h };
}

vec2<u32> Window::size() const noexcept {
	int w, h;
	RGFW_window_getSize(pimpl->window, &w, &h);
	return { w, h };
}

bool Window::is_open() const noexcept { return RGFW_window_shouldClose(pimpl->window) == RGFW_FALSE; }
void Window::close() const noexcept   { RGFW_window_setShouldClose(pimpl->window, true); }

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

#if !defined(FLOYD_RELEASE)
	ImGui_ImplOpenGL3_NewFrame();

	// Update backend for this frame
	ImGuiIO& io = ImGui::GetIO();
	vec2<u32> s = this->size();
	io.DisplaySize = ImVec2(s.x, s.y);
	io.DeltaTime = this->clock.delta();
	vec2<u32> mp = this->mouse_pos();
	io.AddMousePosEvent(mp.x, mp.y);
	io.AddMouseButtonEvent(0, this->mousedown(MouseButton::LEFT));
	io.AddMouseButtonEvent(1, this->mousedown(MouseButton::RIGHT));

	ImGui::NewFrame();
#endif
}

void Window::swap_buffers() const noexcept {
#if !defined(FLOYD_RELEASE)
	ImGui::Begin("Test");
	ImGui::Text("FPS: %.1f", this->fps());
	ImGui::End();


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
	RGFW_window_swapBuffers_OpenGL(pimpl->window);

}

// -- SETTINGS

void Window::set_grab_mouse(const bool state) noexcept {
	if(state) RGFW_window_holdMouse(pimpl->window);
	else RGFW_window_unholdMouse(pimpl->window);
}
void Window::set_title(const std::string &title) noexcept { RGFW_window_setName(pimpl->window, title.c_str()); }
void Window::set_vsync(const bool state) noexcept         { RGFW_window_swapInterval_OpenGL(pimpl->window, state); }
void Window::set_hide_mouse(const bool state) noexcept    { RGFW_window_showMouse(pimpl->window, !state); }
void Window::set_fullscreen(const bool state) noexcept    { RGFW_window_setFullscreen(pimpl->window, state); }
void Window::set_border(const bool state) noexcept        { RGFW_window_setBorder(pimpl->window, state); }
void Window::set_opacity(const u8 opacity) noexcept    { RGFW_window_setOpacity(pimpl->window, opacity); }

void Window::set_min_size(const u32 width, const u32 height) noexcept { RGFW_window_setMinSize(pimpl->window, static_cast<int>(width), static_cast<int>(height)); }
void Window::set_max_size(const u32 width, const u32 height) noexcept { RGFW_window_setMaxSize(pimpl->window, static_cast<int>(width), static_cast<int>(height)); }
void Window::update_viewport(const u32 width, const u32 height) noexcept {
	this->_width = width; this->_height = height;
	// RGFW_window_setAspectRatio(pimpl->window, static_cast<int>(width),
	// 		static_cast<int>(height));
	glViewport(0, 0, width, height);
	this->renderer->update_viewport(width, height);
}

bool Window::is_ui_focused() const noexcept {
#if !defined(FLOYD_RELEASE)
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
#else
	return false;
#endif
}
bool Window::is_mouse_grabbed() const noexcept { return RGFW_window_isHoldingMouse(pimpl->window); }

// -- KEYBOARD

bool Window::keydown(const Keycode key) const noexcept    { return RGFW_isKeyDown(static_cast<u8>(key)); }
bool Window::keypressed(const Keycode key) const noexcept { return RGFW_isKeyPressed(static_cast<u8>(key)); }
bool Window::keyup(const Keycode key) const noexcept      { return RGFW_isKeyReleased(static_cast<u8>(key)); }

// -- MOUSE

bool Window::mousedown(const MouseButton key) const noexcept    { return RGFW_isMouseDown(static_cast<u8>(key)); }
bool Window::mousepressed(const MouseButton key) const noexcept { return RGFW_isMousePressed(static_cast<u8>(key)); }
bool Window::mouseup(const MouseButton key) const noexcept      { return RGFW_isMouseReleased(static_cast<u8>(key)); }

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

vec2<u32> Window::mouse_pos() const noexcept {
	int x, y;
	RGFW_window_getMouse(pimpl->window, &x, &y);
	return { x, y };
}

void Window::enable_ctx() const noexcept  { RGFW_window_makeCurrentContext_OpenGL(pimpl->window); }
void Window::disable_ctx() const noexcept { RGFW_window_makeCurrentContext_OpenGL(nullptr); }

// void Window::mouse_pos_global(int* x, int* y) const noexcept {
// 	RGFW_getGlobalMouse(x, y);
// }

} // namespace floyd
