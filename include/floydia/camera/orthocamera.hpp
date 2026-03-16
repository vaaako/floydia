#pragma once

#include <floydia/camera/camera.hpp>

namespace floyd {

class OrthoCamera final : public Camera {
	public:
		OrthoCamera(const float width, const float height) noexcept;

		// Calculate projection matrix
		glm::mat4 projection() const noexcept override;
		// Calculate view matrix
		inline glm::mat4 view() const noexcept override {
			return glm::mat4(1.0f);
		}

		void set_zoom(const float zoom) noexcept;
	private:
		float zoom = 1.0f;
};

} // namespace floyd
