#pragma once

#include <floydia/camera/camera.hpp>

namespace floyd {

class OrthoCamera final : public Camera {
	public:
		OrthoCamera(const float width, const float height);

		// Calculate projection matrix
		//glm::mat4 view() noexcept;
		// Calculate view matrix
		glm::mat4 projection() noexcept;

		void set_zoom(const float zoom) noexcept;
		void update_width(const float width) noexcept;
		void update_height(const float height) noexcept;
		// Updates width and height
		void update_viewport(const float width, const float height) noexcept;
	private:
		float width;
		float height;
		float zoom = 1.0f;
};

} // namespace floyd
