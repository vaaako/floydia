#include <floydia/camera/orthocamera.hpp>

namespace floyd {

OrthoCamera::OrthoCamera(const float width, const float height) noexcept
	: Camera(width, height) {}

glm::mat4 OrthoCamera::projection() const noexcept {
	if(this->dirty) {
		// top-left origin
		this->proj = glm::ortho(0.0f, this->width, this->height, 0.0f, -1.0f, 1.0f);
		this->dirty = false;
	}
	return this->proj;
}

glm::mat4 OrthoCamera::view() const noexcept {
	const vec3<float> position = this->transform.position(); // cache
	glm::mat4 view = glm::translate(glm::mat4(1.0f),
		glm::vec3(-position.x, -position.y, 0.0f));
		// glm::vec3(-position.x, position.y, 0.0f));
	// Return with zoom applied
	return glm::scale(view, glm::vec3(this->zoom, this->zoom, 1.0f));
}

void OrthoCamera::set_zoom(const float zoom) noexcept {
	this->zoom = zoom;
	this->dirty = true;
}

} // namespace floyd
