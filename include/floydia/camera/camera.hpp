#pragma once

#include <floydia/rendering/transform.hpp>

namespace floyd {

class Camera {
	public:
		struct alignas(16) CameraData {
			glm::mat4 view;
			glm::mat4 proj;
			glm::vec4 camerapos; // on std140 vec3 and vec4 has the same size (16 bytes), but the behaviour may be unexpected (some drivers reads vec3 as 12 bytes), vec4 is safer
		};

	public:
		vec3<float> position = vec3<float>(0.0f);
		vec3<float> forward = vec3<float>(0.0f, 0.0f, -1.0f); // Only used to cheap check when camera moved
	
	public:
		Camera(const float width, const float height) noexcept;

		// Calculate projection matrix
		virtual glm::mat4 projection() const noexcept = 0;
		// Calculate view matrix
		virtual glm::mat4 view() const noexcept = 0;
		// Updates width and height
		void update_viewport(const float width, const float height) noexcept;

	protected:
		mutable glm::mat4 proj;
		float width;
		float height;
		mutable bool dirty = true;
};

} // namespace floyd
