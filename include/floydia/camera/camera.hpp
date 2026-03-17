#pragma once

#include <floydia/gfx/transform.hpp>

namespace floyd {

class Camera {
	public:
		Camera(const float width, const float height) noexcept;

		// Calculate projection matrix
		virtual glm::mat4 projection() const noexcept = 0;
		// Calculate view matrix
		virtual glm::mat4 view() const noexcept = 0;
		// Updates width and height
		void update_viewport(const float width, const float height) noexcept;

	protected:
		mutable glm::mat4 proj; // I think it makes sense this be mutable
		Transform transform;
		float width;
		float height;
		mutable bool dirty = true;
};

} // namespace floyd
