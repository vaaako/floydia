#include <floydia/rendering/transform.hpp>

namespace floyd {

void Transform::set_position(const vec3<float>& position) noexcept {
	this->_position = position;
	this->dirty = true;
	if(this->on_dirty) this->on_dirty();
}

void Transform::set_position(const vec2<float>& position) noexcept {
	this->_position = { position.x, position.y, 0.0f };
	this->dirty = true;
	if(this->on_dirty) this->on_dirty();
}

void Transform::set_scale(const vec3<float>& scale) noexcept {
	this->_scale = scale;
	this->dirty = true;
	if(this->on_dirty) this->on_dirty();
}

void Transform::set_scale(const vec2<float>& scale) noexcept {
	this->_scale = { scale.x, scale.y, 0.0f };
	this->dirty = true;
	if(this->on_dirty) this->on_dirty();
}

void Transform::set_rotation(const vec3<float>& rotation) noexcept {
	this->_euler_degrees = rotation;
	this->_rotation = glm::quat(glm::radians((glm::vec3)rotation));
	this->dirty = true;
	if(this->on_dirty) this->on_dirty();
}

void Transform::set_rotation(const glm::quat& rotation) noexcept {
	this->_rotation = rotation;
	this->dirty = true;
	if(this->on_dirty) this->on_dirty();
}

void Transform::rotate(const vec3<float>& degrees) noexcept {
	this->set_rotation(this->_euler_degrees + degrees);
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
