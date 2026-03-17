#pragma once
#include <string>
#include <memory>
#include <unordered_set>

#include <floydia/types.hpp>
#include <floydia/core/event.hpp>
#include <floydia/core/keycode.hpp>
#include <floydia/core/mousebutton.hpp>
#include <floydia/core/core.hpp>

struct RGFW_window;

namespace floyd {

class Window final {
	public:
		// Reference to 'Core::Renderer' for easy access
		Renderer& renderer;

	public:
		struct Settings {
			uint16 width;
			uint16 height;
			std::string title;
			bool fullscreen = false;
			bool resizable = true;
			bool vsync = false;
		};

		Window(const Settings& settings);
		~Window();

		// Check if the window is open
		bool is_open() const noexcept;
		// Poll all events in this frame
		void poll_events() noexcept;
		// Swap buffers. This is necessary to draw correctly to the window
		void swap_buffers() const noexcept;
		// Clear all previous events
		inline void clear_events() noexcept {
			this->events.clear();
		}
		// Check if a given event has occurred during the frame
		inline bool has_event(const Event event) const noexcept {
			return this->events.find(event) != this->events.end();
		}

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
		// Outputs the current x, y position of the mouse
		void mouse_scroll(float* x, float* y) const noexcept;
		// Outputs the current x, y movement vector of the mouse
		void mouse_vector(float* x, float* y) const noexcept;


		// Outputs the current x, y position of the mouse inside the window
		// void mouse_pos(int* x, int* y) const noexcept;
		// Outputs the current x, y position of the mouse inside the globally
		// void mouse_pos_global(int* x, int* y) const noexcept;

	private:
		struct impl; // hide external libraries from header
		std::unique_ptr<impl> pimpl;

		uint16 width;
		uint16 height;
		std::string title;

		std::unordered_set<Event> events;
};

} // namespace floyd
