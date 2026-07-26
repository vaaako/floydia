#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>

#include "floydia/core/renderer.hpp"
#include "floydia/time/clock.hpp"
#include "floydia/window/event.hpp"
#include "floydia/window/keycode.hpp"
#include "floydia/window/mousebutton.hpp"

#include "floydia/rendering/renderable.hpp"

namespace floyd {

class Window final {
	public:
		// Access renderer
		Renderer* renderer; // Renderer cached to avoid repeated lookup

	public:
		struct Settings {
			u32 width;
			u32 height;
			std::string title;
			// NOT IMPLEMENTED YET
			bool fullscreen = false;
			bool resizable = true;
		};

	public:
		Window(const Settings& settings);
		~Window();

		// Window current width
		inline u32 width() const noexcept { return this->_width; }
		// Window current height
		inline u32 height() const noexcept { return this->_height; }
		// Window current size
		vec2<u32> sttc_size() noexcept;
		// Window current size
		vec2<u32> size() const noexcept;

		// Check if the window is open
		bool is_open() const noexcept;
		// Mark window as "should close"
		void close() const noexcept;
		// Poll all events in this frame.
		// Call this at the beginning of each frame
		void poll_events() noexcept;
		// Swap buffers. This is necessary to draw correctly to the window
		void swap_buffers() const noexcept;
		// Check if a given event has occurred during the frame
		inline bool has_event(const Event event) const noexcept {
			return this->events.find(event) != this->events.end();
		}
		// Run a callback for given event
		inline void on_event(const Event event,
				const std::function<void()>& callback) noexcept {
			this->events_callbacks.emplace(event, callback);
		}

		// Elapsed time between frames in second.
		// ~0.016 -> 16ms (60 FPS).
		// ~0.033 -> 33ms (30 FPS)
		inline float dt() const noexcept { return this->clock.delta(); }

		inline float fps() const noexcept {
			const float dt = this->dt();
			return (dt <= 0.0f) ? 0.0f : 1.0f / dt;
		}

		// void set_icon(const Image& image) noexcept;
		// void set_mouse(RGFW_mouse*) noexcept;

		// Renames the window
		void set_title(const std::string &title) noexcept;
		void set_vsync(const bool state) noexcept;
		// Lock/Unlock the cursor
		void set_grab_mouse(const bool state) noexcept;
		// Hide/Unhide the cursor.
		// While held, captures move vector
		void set_hide_mouse(const bool state) noexcept;
		// Toggles window borders
		void set_border(const bool state) noexcept;
		// Fullscreen mode for the window
		void set_fullscreen(const bool state) noexcept;
		// Sets the opacity level of the window
		void set_opacity(const u8 opacity) noexcept;

		// Sets the minimum size of the window
		void set_min_size(const u32 width, const u32 height) noexcept;
		// Sets the maximum size of the window
		void set_max_size(const u32 width, const u32 height) noexcept;
		// Resizes the window viewport to the given dimensions
		void update_viewport(const u32 width, const u32 height) noexcept;

		// Returns true if mouse is grabbed
		bool is_mouse_grabbed() const noexcept;
	#if !defined(FLOYD_NO_EDITOR_PANEL)
		// Returns true if keyboard clicked inside UI
		bool ui_key_clicked() const noexcept;
		// Returns true if mouse clicked inside UI
		bool ui_mouse_clicked() const noexcept;
	#endif

		// Open an ImGui window to edit a Renderable
		void editor_panel(Renderable* obj) const noexcept;

		// NOTE: When typing on ImGui. Keys will be handled by RGFW too
		// but i dont think it is really necessary to handle this case

		// Returns true if the key is down
		bool keydown(const Keycode key) const noexcept;
		// Returns true if the key is pressed during the frame
		bool keypressed(const Keycode key) const noexcept;
		// Returns true if the key was released during the frame
		bool keyup(const Keycode key) const noexcept;

		// Returns true if the Mouse Button is down
		bool mousedown(const MouseButton key) const noexcept;
		// Returns true if the Mouse Button is pressed during the frame
		bool mousepressed(const MouseButton key) const noexcept;
		// Returns true if the Mouse Button was released during the frame
		bool mouseup(const MouseButton key) const noexcept;
		// Returns mouse scroll (+1: up / -1: down)
		vec2<float> mouse_scroll() const noexcept;
		// Outputs the delta x, y position of the mouse inside the window
		vec2<float> mouse_vector() const noexcept;
		// Outputs the current x, y position of the mouse inside the window
		vec2<u32> mouse_pos() const noexcept;

		// Make this window renderable
		void enable_ctx() const noexcept;
		// Make this window not renderable
		void disable_ctx() const noexcept;

	private:
		struct impl; // hide external libraries from header
		std::unique_ptr<impl> pimpl;

		// Registered callbacks for events
		std::unordered_map<Event, const std::function<void()>> events_callbacks;
		// Used to count DT and FPS
		Clock clock = Clock();

		std::string title;
		u32 _width;
		u32 _height;

		int mouse_x = 0;
		int mouse_y = 0;

		std::unordered_set<Event> events;
};

} // namespace floyd
