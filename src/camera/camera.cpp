#include <floydia/camera/camera.hpp>

#include <stdexcept>

namespace floyd {

Camera::Camera(const float width, const float height) noexcept
		: width(width), height(height) {

	if(width == 0 || height == 0) {
		std::invalid_argument("'width' or 'height' is 0");
	}
}

// Updates width and height
void Camera::viewport(const float width, const float height) noexcept {
	this->width = width;
	this->height = height;
	this->dirty = true;
}

} // namespace floyd
