#include <floydia/camera/perspectivecamera.hpp>

namespace floyd {

PerspectiveCamera::PerspectiveCamera(const float fov, const float width, const float height) noexcept
	: Camera(width, height), fov(fov) {}

glm::mat4 PerspectiveCamera::projection() noexcept {
	if(this->dirty) {
		this->proj = glm::perspective(
			glm::radians(this->fov), (this->width / this->height),
			this->near_plane, this->far_plane
		);
		this->dirty = false;
	}
	return this->proj;
}

} // namespace floyd
