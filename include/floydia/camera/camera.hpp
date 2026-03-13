#pragma once

#include <floydia/gfx/transform.hpp>

namespace floyd {

class Camera {
	public:
		Camera() = default;

		// TODO
		inline glm::mat4 view()  {
			return glm::inverse(this->transform.model_matrix());
		}
		//inline glm::mat4 get_view_matrix() const noexcept override {
		//	return glm::lookAt(this->position, this->position + this->forward, this->up);
		//}
		// TODO
		// Calculate view matrix
		//virtual glm::mat4 view() const noexcept = 0;
	protected:
		glm::mat4 proj;
		Transform transform;
		bool dirty = true;
};

} // namespace floyd
