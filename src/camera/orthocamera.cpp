#include <floydia/camera/orthocamera.hpp>

namespace floyd {

OrthoCamera::OrthoCamera(const float width, const float height) noexcept
	: Camera(width, height) {}

glm::mat4 OrthoCamera::projection() noexcept {
	if(this->dirty) {
		// left, right, bottom, top
		this->proj = glm::ortho(0.0f, this->width, 0.0f, this->height);
		// left, right, bottom, top, zNear, zFar
		//this->proj = glm::ortho(0.0f, this->width, this->height, 0.0f, -1.0f, 1.0f);
		this->proj = glm::scale(this->proj, glm::vec3(this->zoom, this->zoom, 1.0f));
	}
	return this->proj;
}

void OrthoCamera::set_zoom(const float zoom) noexcept {
	this->zoom = zoom;
	this->dirty = true;
}

} // namespace floyd
