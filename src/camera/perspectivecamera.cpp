#include <floydia/camera/perspectivecamera.hpp>

namespace floyd {

glm::mat4 PerspectiveCamera::projection(const float aspect) noexcept {
	if(this->dirty) {
		this->proj = glm::perspective(glm::radians(this->fov),
			aspect,
			this->near_plane, this->far_plane
		);
	}
	return this->proj;
}

} // namespace floyd
