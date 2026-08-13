#pragma once

#include <floydia/camera/camera.hpp>

namespace floyd {

class PerspectiveCamera final : public Camera {
	public:
		vec3<float> up     = vec3<float>(0.0f, 1.0f,  0.0f);
		vec3<float> right  = vec3<float>(1.0f, 0.0f,  0.0f);
		// Horizontal
		float yaw = -90.0f; // forward: -Z
		// Vertical
		float pitch = 0.0f;
		// Camera sensitivity
		float sensitivity = 0.1f;

	public:
		PerspectiveCamera(const float fov, const float width, const float height) noexcept;

		// Calculate projection matrix
		glm::mat4 proj() const noexcept override;
		// Calculate view matrix
		inline glm::mat4 view() const noexcept override {
			return glm::lookAt(this->position, this->position + this->forward, this->up);
		}

		// Takes a normalized vector and returns world dir
		vec3<float> calc_move_dir(const vec3<float>& dir) noexcept;
		// Free-fly camera
		void free_fly(const vec3<float>& dir, const float velocity) noexcept;
		// Takes a delta vector and rotates the cameras
		void rotate(const vec2<float>& delta) noexcept;
		// Updates camera's vectors using yaw and pitch
		void update_vectors() noexcept;

		inline float get_fov() const noexcept {
			return this->fov;
		}

		inline void set_fov(const float fov) noexcept {
			this->fov = fov;
			this->dirty = true;
		}

		// How near the camera will render
		inline float get_near_plane() const noexcept {
			return this->near_plane;
		}

		// Set how near the camera will render
		inline void set_near_plane(const float near_plane) noexcept {
			this->near_plane = near_plane;
			this->dirty = true;
		}

		// How far the camera will render
		inline float get_far_plane() const noexcept {
			return this->far_plane;
		}

		// Set how far the camera will render
		inline void set_far_plane(const float far_plane) noexcept {
			this->far_plane = far_plane;
			this->dirty = true;
		}

	private:
		float fov;
		float near_plane = 0.1f;
		float far_plane  = 1000.0f;
};

} // namespace floyd
