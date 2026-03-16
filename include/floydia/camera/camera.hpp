#pragma once

#include <floydia/gfx/transform.hpp>

namespace floyd {

class Camera {
	public:
		Camera(const float width, const float height) noexcept;

		// Calculate projection matrix
		virtual glm::mat4 projection() noexcept = 0;

		// Calculate view matrix
		virtual glm::mat4 view() const noexcept = 0;
		// return glm::inverse(this->transform.model_matrix());

		void update_width(const float width) noexcept;
		void update_height(const float height) noexcept;
		// Updates width and height
		void update_viewport(const float width, const float height) noexcept;

	protected:
		glm::mat4 proj;
		Transform transform;
		float width;
		float height;
		bool dirty = true;
};

} // namespace floyd
