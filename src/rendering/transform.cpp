#include <floydia/rendering/transform.hpp>

namespace floyd {

void Transform::set_position(const vec3<float>& position) noexcept {
	this->_position = position;
	this->dirty = true;
}

void Transform::set_scale(const vec3<float>& scale) noexcept {
	this->_scale = scale;
	this->dirty = true;
}

void Transform::set_rotation(const vec3<float>& rotation) noexcept {
	this->_rotation = rotation;
	this->dirty = true;
}

const glm::mat4& Transform::model_matrix() const noexcept {
	if(this->dirty) {
		glm::mat4 T = glm::translate(glm::mat4(1.0f), this->_position);
		glm::mat4 R = glm::mat4_cast(this->_rotation); // glm::toMat4
		glm::mat4 S = glm::scale(glm::mat4(1.0f), this->_scale);

		this->cached_matrix = T * R * S;
		this->dirty = false;
	}
	return this->cached_matrix;
}

} // namespace floyd
