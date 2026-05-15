#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>

#include <floydia/time/clock.hpp>
#include <floydia/window/event.hpp>
#include <floydia/window/keycode.hpp>
#include <floydia/window/mousebutton.hpp>

#include <floydia/camera/camera.hpp>
#include <floydia/rendering/renderable.hpp>
#include <floydia/geometry/cube.hpp>
#include <floydia/core/core.hpp>

namespace floyd {

class Window final {
	public:
		struct Settings {
			uint32 width;
			uint32 height;
			std::string title;
			// NOT IMPLEMENTED YET
			bool fullscreen = false;
			bool resizable = true;
		};

		Window(const Settings& settings);
		~Window();

		// Window current width
		inline uint32 width() const noexcept { return this->_width; }
		// Window current height
		inline uint32 height() const noexcept { return this->_height; }
		// Window current size
		vec2<uint32> sttc_size() noexcept;
		// Window current size
		vec2<uint32> size() const noexcept;

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
				const std::function<void()> callback) noexcept {
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
		void set_opacity(const uint8 opacity) noexcept;

		// Sets the minimum size of the window
		void set_min_size(const uint32 width, const uint32 height) noexcept;
		// Sets the maximum size of the window
		void set_max_size(const uint32 width, const uint32 height) noexcept;
		// Resizes the window viewport to the given dimensions
		void viewport(const uint32 width, const uint32 height) noexcept;

		// Returns true if mouse is grabbed
		bool is_mouse_grabbed() const noexcept;

		// Clears screen
		inline void clear() const { renderer->clear(); }

		// Advances 'frameindex' and waits for the GPU to finish reading
		// the current frame's buffer slots before the CPU writes new data
		void begin_frame() noexcept { renderer->begin_frame(); }
		// Singals GPU fences to mark this frame's buffer slots as in-flight,
		// preventing the CPU from overwriting them before the GPU is done
		void end_frame() noexcept { renderer->end_frame(); }
		// Advances the frame index, syncs GPU fences, updates camera UBO,
		// updates the frustum, and rebuilds persistent batches if dirty
		inline void begin_draw(const Camera& camera) const noexcept { renderer->begin_draw(*this, camera); }
		
		// Includes persistent objects in the current pass.
		// Must be called after 'begin_draw()' and before 'flush()'.
		// Persistent objects are only rebuilt when dirty (i.e. when 'add()' is called).
		// Without this call, persistent objects are excluded from the current pass
		void draw_persistent() const noexcept { renderer->draw_persistent(); }
		// Submit a dynamic object for this frame. Frustum culled
		inline void draw(Renderable& obj) const noexcept { renderer->draw(obj); }
		// Submit a persistent object. Batched once and reused every frame.
		// Skips per-frame frustum culling
		inline void add(const Renderable& obj) const noexcept { renderer->add(obj); }
		// Submit a dynamic light object for this frame
		inline void draw(const Light& light) const noexcept { renderer->draw(light); }
		// Submit a persistent light object
		inline void add(const Light& light) const noexcept { renderer->add(light); }
		// Upload instance data to SSBo and issue draw calls
		inline void flush() const { renderer->flush(); }

		// Submit a text object for this frame
		// void draw_text(const std::string& text, const vec2<float>& pos, const std::shared_ptr<Text>& font,
		// 	const float scale = 1.0f, const vec4<float>& color = vec4<float>(1.0f)) noexcept { renderer->draw_text(text, pos, font, scale, color); }


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
		vec2<int> mouse_pos() const noexcept;

		// Make this window renderable
		void enable_ctx() const noexcept;
		// Make this window not renderable
		void disable_ctx() const noexcept;

	private:
		Renderer* renderer; // Renderer cached to avoid repeated lookup

		struct impl; // hide external libraries from header
		std::unique_ptr<impl> pimpl;

		// Registered callbacks for events
		std::unordered_map<Event, const std::function<void()>> events_callbacks;
		// Used to count DT and FPS
		Clock clock = Clock();

		std::string title;
		uint32 _width;
		uint32 _height;

		int mouse_x = 0;
		int mouse_y = 0;

		std::unordered_set<Event> events;
};

} // namespace floyd
