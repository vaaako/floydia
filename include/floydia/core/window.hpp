#pragma once
#include <string>
#include <memory>
#include <unordered_set>

#include <floydia/types.hpp>
#include <floydia/core/event.hpp>
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
		// Check if a given event has occurred in this frame
		inline bool has_event(const Event event) const noexcept {
			return this->events.find(event) != this->events.end();
		}

	private:
		struct impl; // hide external libraries from header
		std::unique_ptr<impl> pimpl;

		uint16 width;
		uint16 height;
		std::string title;

		std::unordered_set<Event> events;
};

} // namespace floyd
