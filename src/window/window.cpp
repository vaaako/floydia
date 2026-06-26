#include "floydia/helpers/logger.hpp"
#include "floydia/window/window.hpp"
#include "floydia/core/core.hpp"

#include <mutex>

#include "floydia/helpers/ui.hpp"
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
		RGFW_windowCenter | RGFW_windowOpenGL
	);

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

	TRACELOG(logger::Info, "Window (%p) initialized! (%d total)", (void*)pimpl->window, ++wincount);

	// Release context (if not first window). User must manually set current context
	if(wincount > 1) this->disable_ctx();
	this->renderer = Core::get().renderer.get();

	// Disable VSync by default
	RGFW_window_swapInterval_OpenGL(pimpl->window, RGFW_FALSE);
}

Window::~Window() {
	std::lock_guard<std::mutex> lock(wmutex);

	if(pimpl->window != nullptr) {
		TRACELOG(logger::Info, "Window (%p) closed! (%d remaining)", (void*)pimpl->window, --wincount);
		if(wincount == 0) {
			TRACELOG(logger::Info, "OpenGL context destroyed!");
			// this->enable_ctx();
			this->disable_ctx();
			shared_context = nullptr;
			// Delete ImGui
			TRACELOG(logger::Info, "ImGui destroyed!");
			ImGui_ImplOpenGL3_Shutdown();
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

#if !defined(FLOYD_RELEASE)
	ImGuiIO& io = ImGui::GetIO();
#endif

	while(RGFW_window_checkEvent(pimpl->window, &event)) {
		this->events.emplace(static_cast<Event>(event.type));

	#if !defined(FLOYD_RELEASE)
		// Update ImGui events
		if(event.type == RGFW_keyPressed) {
			// Add pressed key to ImGui events
			io.AddKeyEvent(ui::KeyToImGuiKey(event.key.value), true);
			io.AddInputCharacter(event.key.sym);
		} else if(event.type == RGFW_keyReleased) {
			io.AddKeyEvent(ui::KeyToImGuiKey(event.key.value), false);
		}
	#endif

		// Check for registered callback and run it
		if(!this->events_callbacks.empty()) {
			auto it = this->events_callbacks.find(static_cast<Event>(event.type));
			if(it != this->events_callbacks.end()) {
				it->second();
			}
		}
	}

// Update mouse
#if !defined(FLOYD_RELEASE)
	ImGui_ImplOpenGL3_NewFrame();

	const vec2<u32> s = this->size();
	const vec2<u32> mp = this->mouse_pos();
	const vec2<float> scroll = this->mouse_scroll();

	// Update window
	io.DisplaySize = ImVec2(s.x, s.y);
	io.DeltaTime = this->clock.delta();
	// Update mouse
	io.AddMousePosEvent(mp.x, mp.y);
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
#if !defined(FLOYD_RELEASE)
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

bool Window::is_mouse_grabbed() const noexcept { return RGFW_window_isHoldingMouse(pimpl->window); }

// -- Editor Panel

void Window::editor_panel(Renderable* obj) const noexcept {
#if defined(FLOYD_RELEASE)
	(void)(obj);
	return;
#else
	ImGui::Begin("Properties");
	if(obj == nullptr) {
		ImGui::TextDisabled("No object selected");
		ImGui::End();
		return;
	}

	ImGui::Text("UUID: %lu", obj->uuid());
	if(!obj->name.empty()) ImGui::Text("Name: %s", obj->name.c_str());
	ImGui::Separator();

	// Material
	bool material_changed = false; // If material changed, entire batch needs to change
	if(ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
		const vec4<float> color = obj->color_norm();
		float c[4] = { color.x, color.y, color.z, color.w };
		if(ui::color_edit_scroll4("Color", c)) { obj->set_color_norm({ c[0], c[1], c[2], c[3] }); material_changed = true; }

		MaterialInstance& mat = *obj->material(); // cache
		float metallic = mat.metallic;
		float roughness = mat.roughness;
		if(ui::drag_scroll_float("Metallic", &metallic, 0.1f, 0.0f, 1.0f))   { mat.metallic = metallic; material_changed = true; }
		if(ui::drag_scroll_float("Roughness", &roughness, 0.1f, 0.0f, 1.0f)) { mat.roughness = roughness; material_changed = true; }
	}
	
	ImGui::Separator();

	// Transform
	if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		const vec3<float> pos = obj->transform.position();
		float p[3] = { pos.x, pos.y, pos.z };
		if(ui::drag_scroll_float3("Position", p, 0.1f,
			std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
			obj->transform.set_position({ p[0], p[1], p[2] });
		}

		// Euler to rotation
		const vec3<float> euler = obj->transform.euler_degrees();
		float r[3] = { euler.x, euler.y, euler.z };
		if(ui::drag_scroll_float3("Rotation", r, 0.25f, -360.0f, 360.0f)) {
			obj->transform.set_rotation({ r[0], r[1], r[2] });
		}
	}

	ImGui::Separator();

	if(ImGui::CollapsingHeader("Textures")) {
		static std::shared_ptr<Texture> selected = nullptr;
		if(selected == nullptr) selected = obj->material()->albedo;

		// 64: image size
		// 16: offset
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 8.0f); // default is 14
		ImGui::BeginChild("##tex_scroll", ImVec2(0, 64 + 16), false, ImGuiWindowFlags_HorizontalScrollbar);
		for(auto& [_, texentry] : assets().textures) {
			std::shared_ptr<Texture> tex = texentry.texture; // cache

			ImGui::Image((ImTextureID)(intptr_t)texentry.texture->id(), ImVec2(64, 64),
				ImVec2(0, 1), ImVec2(1, 0) // Display upside down, which makes correct
			);
			if(ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("%s", texentry.path.c_str());
				ImGui::EndTooltip();
			}
			if(ImGui::IsItemClicked()) { selected = tex; obj->material()->albedo = tex; material_changed = true; }
			ImGui::SameLine();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImGui::Separator();
		ImGui::Text("Filter:");
		if(ImGui::Button("Nearest")) selected->set_filter(Texture::Filter::Nearest);
		ImGui::SameLine();
		if(ImGui::Button("Linear")) selected->set_filter(Texture::Filter::Linear);
		ImGui::Separator();
		ImGui::Text("Wrap:");
		if(ImGui::Button("Repeat")) selected->set_filter(Texture::Filter::Repeat);
		ImGui::SameLine();
		if(ImGui::Button("Clamp")) selected->set_filter(Texture::Filter::Clamp);
		ImGui::SameLine();
		if(ImGui::Button("Mirrored")) selected->set_filter(Texture::Filter::Mirrored);
	}

	// Transform changes trigger 'on_dirty' callback
	if(material_changed && obj->is_persistent) renderer->mark_dirty();
	ImGui::End();
#endif
}

#if !defined(FLOYD_RELEASE)
bool Window::ui_key_clicked() const noexcept {
	return ImGui::GetIO().WantCaptureMouse;
}

bool Window::ui_mouse_clicked() const noexcept {
	return ImGui::GetIO().WantCaptureKeyboard;
}
#endif


// -- KEYBOARD

bool Window::keydown(const Keycode key) const noexcept    { return RGFW_isKeyDown(static_cast<u8>(key)); }
bool Window::keypressed(const Keycode key) const noexcept { return RGFW_isKeyPressed(static_cast<u8>(key)); }
bool Window::keyup(const Keycode key) const noexcept      { return RGFW_isKeyReleased(static_cast<u8>(key)); }

// -- MOUSE

bool Window::mousedown(const MouseButton key) const noexcept {
#if !defined(FLOYD_RELEASE)
	if(ImGui::GetIO().WantCaptureMouse) return false;
#endif
	return RGFW_isMouseDown(static_cast<u8>(key));
}
bool Window::mousepressed(const MouseButton key) const noexcept {
#if !defined(FLOYD_RELEASE)
	if(ImGui::GetIO().WantCaptureMouse) return false;
#endif
	return RGFW_isMousePressed(static_cast<u8>(key));
}
bool Window::mouseup(const MouseButton key) const noexcept {
#if !defined(FLOYD_RELEASE)
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

void Window::enable_ctx() const noexcept  { RGFW_window_makeCurrentContext_OpenGL(pimpl->window); }
void Window::disable_ctx() const noexcept { RGFW_window_makeCurrentContext_OpenGL(nullptr); }

} // namespace floyd
