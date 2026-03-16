#pragma once

#include <floydia/camera/camera.hpp>

/*
void Camera::update_fps(const Input& input, float dt)
{
    // mouse look
    transform.rotate_y(input.mouse_delta_x() * dt);
    transform.rotate_x(input.mouse_delta_y() * dt);

    // WASD movement
    glm::vec3 forward = transform.forward();
    glm::vec3 right   = transform.right();

    if (input.key_down(KEY_W))
        transform.translate(forward * dt * 5.0f);

    if (input.key_down(KEY_S))
        transform.translate(-forward * dt * 5.0f);

    if (input.key_down(KEY_A))
        transform.translate(-right * dt * 5.0f);

    if (input.key_down(KEY_D))
        transform.translate(right * dt * 5.0f);
}
*/

namespace floyd {

class PerspectiveCamera final : public Camera {
	public:
		PerspectiveCamera(const float fov, const float width, const float height) noexcept;

		// Calculate projection matrix
		glm::mat4 projection() const noexcept override;
		// Calculate view matrix
		inline glm::mat4 view() const noexcept override {
			return glm::inverse(this->transform.model_matrix());
			// return glm::lookAt(this->position, this->position + this->forward, this->up);
		}

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

		// Changes camera's height in cm
		//inline set_height(const float height) noexcept {
		//	this->transform.position.y = std::max(height * 0.01f, 0.0f);
		//}

	private:
		// debug
		vec3<float> position = vec3<float>(0.0f, 0.0f, 0.0f);
		vec3<float> up       = vec3<float>(0.0f, 1.0f, 0.0f);
		vec3<float> forward  = vec3<float>(0.0f, 0.0f, -1.0f);
		vec3<float> right    = vec3<float>(1.0f, 0.0f, 0.0f);

		float fov;
		float near_plane = 0.1f;
		float far_plane  = 1000.0f;
};

} // namespace floyd
