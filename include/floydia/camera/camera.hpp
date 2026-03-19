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
		void viewport(const float width, const float height) noexcept;

	protected:
		mutable glm::mat4 proj;
		vec3<float> position = vec3<float>(0.0f);
		float width;
		float height;
		mutable bool dirty = true;
};

} // namespace floyd
