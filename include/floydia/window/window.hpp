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
		struct Settings {
			u32 width;
			u32 height;
			std::string title;
			bool fullscreen = false;
			bool resizable = true;
		};

		std::unique_ptr<Renderer> renderer;

	public:
		Window(const Settings& settings);
		~Window();

		// Current window width
		inline u32 width() const noexcept { return this->s_width; }
		// Current window height
		inline u32 height() const noexcept { return this->s_height; }
		// Current window size
		vec2<u32> size() const noexcept;
		// Current window size, without instance
		// static inline vec2<u32> static_size() noexcept { return { this->s_width, this->s_height }; }

		// True if the window should stay open
		bool is_open() const noexcept;
		// Marks the window to close
		void close() const noexcept;
		// Polls all events for this frame. Call at the start of each frame
		void poll_events() noexcept;
		// Swaps buffers. Required to present the frame
		void swap_buffers() const noexcept;

		// True if 'event' happened this frame
		inline bool has_event(const Event event) const noexcept { return this->events.find(event) != this->events.end(); }
		// Registers a callback for 'event'. Multiple callbacks per event are allowed
		inline void on_event(const Event event, const std::function<void()>& callback) noexcept { this->events_callbacks[event].push_back(callback); }

		// Time between frames, in seconds.
		// ~0.016 -> 16ms (60 FPS)
		// ~0.033 -> 33ms (30 FPS)
		inline float dt() const noexcept { return this->clock.delta(); }
		// Frames per second, derived from dt()
		inline float fps() const noexcept {
			const float dt = this->dt();
			return (dt <= 0.0f) ? 0.0f : 1.0f / dt;
		}

		
		void set_title(const std::string& title) noexcept;
		void set_vsync(const bool state) noexcept;
		void set_grab_mouse(const bool state) noexcept;
		void set_hide_mouse(const bool state) noexcept;
		// Toggles window borders
		void set_border(const bool state) noexcept;
		void set_fullscreen(const bool state) noexcept;
		void set_opacity(const u8 opacity) noexcept;
		void set_min_size(const u32 width, const u32 height) noexcept;
		void set_max_size(const u32 width, const u32 height) noexcept;
		// Resizes the viewport (both GL and the renderer's cached size)
		void update_viewport(const u32 width, const u32 height) noexcept;

		bool is_mouse_grabbed() const noexcept;

		bool keydown(const Keycode key) const noexcept;
		bool keypressed(const Keycode key) const noexcept;
		bool keyup(const Keycode key) const noexcept;

		bool mousedown(const MouseButton key) const noexcept;
		bool mousepressed(const MouseButton key) const noexcept;
		bool mouseup(const MouseButton key) const noexcept;

		// Scroll delta this frame (+1 up / -1 down)
		vec2<float> mouse_scroll() const noexcept;
		// Mouse movement delta this frame
		vec2<float> mouse_vector() const noexcept;
		// Mouse position in window space
		vec2<u32> mouse_pos() const noexcept;

		// Makes this window's GL context current
		void enable_gl_ctx() const noexcept;
		// Releases the current GL context
		void disable_gl_ctx() const noexcept;

	private:
		struct impl; // hides RGFW from the header
		std::unique_ptr<impl> pimpl;

		std::unordered_map<Event, std::vector<std::function<void()>>> events_callbacks;
		std::unordered_set<Event> events;

		std::string title;
		Clock clock = Clock();

		int mouse_x = 0;
		int mouse_y = 0;

		static inline u32 s_width = 0;
		static inline u32 s_height = 0;
};

} // namespace floyd
