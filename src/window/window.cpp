#include "floydia/helpers/logger.hpp"
#include "floydia/window/window.hpp"
#include "floydia/core/core.hpp"

#if defined(FLOYD_EDITOR_PANEL)
#include "imgui/imgui_impl_opengl3.h"
#include "floydia/helpers/ui.hpp"
#endif

#include "floydia/rgfwimpl.hpp"

// https://github.com/ColleagueRiley/RGFW/blob/main/examples/multi-window/multi-window.c

namespace floyd {

struct Window::impl {
	RGFW_window* window;
};

Window::Window(const Window::Settings& settings)
	: pimpl(std::make_unique<impl>()),
	title(settings.title) {

	this->s_width = settings.width;
	this->s_height = settings.height;

	// Initialize Window
	pimpl->window = RGFW_createWindow(
		settings.title.c_str(), 0, 0, settings.width, settings.height,
		RGFW_windowCenter | RGFW_windowOpenGL
	);

	if(pimpl->window == NULL) {
		TRACELOG(logger::Error, "Failed to create window!");
		return;
	}

	if(!settings.resizable) {
		RGFW_window_setFlags(pimpl->window, RGFW_windowNoResize);
	}

	this->enable_gl_ctx();
	Core::get().initialize();

#if defined(FLOYD_EDITOR_PANEL)
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Initialize backend for this application
	ImGuiIO& io = ImGui::GetIO();
	io.BackendPlatformName = "floyd_rgfw";
	// Initialize OpenGL backend
	ImGui_ImplOpenGL3_Init("#version 460");
	TRACELOG(logger::Info, "ImGui initialized!");
#endif

	this->renderer = std::make_unique<Renderer>();
	this->update_viewport(settings.width, settings.height);
	this->set_vsync(false);

	TRACELOG(logger::Info, "Window (%p) initialized!", (void*)pimpl->window);
}

Window::~Window() {
	if(pimpl->window != nullptr) {
		TRACELOG(logger::Info, "Window (%p) closed!", (void*)pimpl->window);

	#if defined(FLOYD_EDITOR_PANEL)
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext();
		TRACELOG(logger::Info, "ImGui destroyed!");
	#endif

		RGFW_window_close(pimpl->window); // Automatically calls RGFW_window_deleteContext_OpenGL
		pimpl->window = nullptr;
	}
}

vec2<u32> Window::size() const noexcept {
	int w, h;
	RGFW_window_getSize(pimpl->window, &w, &h);
	return { w, h };
}

bool Window::is_open() const noexcept { return RGFW_window_shouldClose(pimpl->window) == RGFW_FALSE; }
void Window::close() const noexcept   { RGFW_window_setShouldClose(pimpl->window, true); }

void Window::poll_events() noexcept {
	this->events.clear();
	this->clock.mark(); // Mark new delta time

	RGFW_event event;

#if defined(FLOYD_EDITOR_PANEL)
	ImGuiIO& io = ImGui::GetIO();
#endif

	while(RGFW_window_checkEvent(pimpl->window, &event)) {
		const Event ev = static_cast<Event>(event.type);
		this->events.emplace(ev);

	#if defined(FLOYD_EDITOR_PANEL)
		// ImGui key events
		if(event.type == RGFW_keyPressed) {
			io.AddKeyEvent(ui::KeyToImGuiKey(event.key.value), true);
			io.AddInputCharacter(event.key.sym);
		} else if(event.type == RGFW_keyReleased) {
			io.AddKeyEvent(ui::KeyToImGuiKey(event.key.value), false);
		}
	#endif

		// Run every registered callback for this event
		auto it = this->events_callbacks.find(ev);
		if(it != this->events_callbacks.end()) {
			for(const std::function<void()>& cb : it->second) cb();
		}
	}

#if defined(FLOYD_EDITOR_PANEL)
	ImGui_ImplOpenGL3_NewFrame();

	const vec2<u32> s = this->size();
	const vec2<u32> mp = this->mouse_pos();
	const vec2<float> scroll = this->mouse_scroll();

	// Update window
	io.DisplaySize = ImVec2((float)s.x, (float)s.y);
	io.DeltaTime = this->clock.delta();
	// Update mouse
	io.AddMousePosEvent((float)mp.x, (float)mp.y);
	io.AddMouseWheelEvent(scroll.x, scroll.y);
	io.AddMouseButtonEvent(0, RGFW_isMouseDown(RGFW_mouseLeft));
	io.AddMouseButtonEvent(1, RGFW_isMouseDown(RGFW_mouseRight));
	// Precision keys for slides
	io.AddKeyEvent(ImGuiMod_Shift, RGFW_isKeyDown(RGFW_shiftL) || RGFW_isKeyDown(RGFW_shiftR));
	io.AddKeyEvent(ImGuiMod_Ctrl,  RGFW_isKeyDown(RGFW_controlL) || RGFW_isKeyDown(RGFW_controlR));
	io.AddKeyEvent(ImGuiMod_Alt,   RGFW_isKeyDown(RGFW_altL) || RGFW_isKeyDown(RGFW_altR));

	ImGui::NewFrame();
#endif
}

void Window::swap_buffers() const noexcept {
#if defined(FLOYD_EDITOR_PANEL)
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
	RGFW_window_swapBuffers_OpenGL(pimpl->window);
}


void Window::set_title(const std::string& title) noexcept { RGFW_window_setName(pimpl->window, title.c_str()); }
void Window::set_vsync(const bool state) noexcept         { RGFW_window_swapInterval_OpenGL(pimpl->window, state); }
void Window::set_grab_mouse(const bool state) noexcept {
	if(state) RGFW_window_holdMouse(pimpl->window);
	else RGFW_window_unholdMouse(pimpl->window);
}
void Window::set_hide_mouse(const bool state) noexcept { RGFW_window_showMouse(pimpl->window, !state); }
void Window::set_border(const bool state) noexcept     { RGFW_window_setBorder(pimpl->window, state); }
void Window::set_fullscreen(const bool state) noexcept { RGFW_window_setFullscreen(pimpl->window, state); }
void Window::set_opacity(const u8 opacity) noexcept    { RGFW_window_setOpacity(pimpl->window, opacity); }
void Window::set_min_size(const u32 width, const u32 height) noexcept { RGFW_window_setMinSize(pimpl->window, (int)width, (int)height); }
void Window::set_max_size(const u32 width, const u32 height) noexcept { RGFW_window_setMaxSize(pimpl->window, (int)width, (int)height); }

void Window::update_viewport(const u32 width, const u32 height) noexcept {
	this->s_width = width; this->s_height = height;
	this->renderer->update_viewport(width, height);
	glViewport(0, 0, width, height);
}

bool Window::is_mouse_grabbed() const noexcept { return RGFW_window_isHoldingMouse(pimpl->window); }


bool Window::keydown(const Keycode key) const noexcept    { return RGFW_isKeyDown(static_cast<u8>(key)); }
bool Window::keypressed(const Keycode key) const noexcept { return RGFW_isKeyPressed(static_cast<u8>(key)); }
bool Window::keyup(const Keycode key) const noexcept      { return RGFW_isKeyReleased(static_cast<u8>(key)); }

bool Window::mousedown(const MouseButton key) const noexcept {
#if defined(FLOYD_EDITOR_PANEL)
	if(ImGui::GetIO().WantCaptureMouse) return false;
#endif
	return RGFW_isMouseDown(static_cast<u8>(key));
}
bool Window::mousepressed(const MouseButton key) const noexcept {
#if defined(FLOYD_EDITOR_PANEL)
	if(ImGui::GetIO().WantCaptureMouse) return false;
#endif
	return RGFW_isMousePressed(static_cast<u8>(key));
}
bool Window::mouseup(const MouseButton key) const noexcept {
#if defined(FLOYD_EDITOR_PANEL)
	if(ImGui::GetIO().WantCaptureMouse) return false;
#endif
	return RGFW_isMouseReleased(static_cast<u8>(key));
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

vec2<u32> Window::mouse_pos() const noexcept {
	int x, y;
	RGFW_window_getMouse(pimpl->window, &x, &y);
	return { x, y };
}

void Window::enable_gl_ctx() const noexcept  { RGFW_window_makeCurrentContext_OpenGL(pimpl->window); }
void Window::disable_gl_ctx() const noexcept { RGFW_window_makeCurrentContext_OpenGL(nullptr); }

} // namespace floyd
