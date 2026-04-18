#pragma once

#include <floydia/types.hpp>

namespace floyd {

// This class takes care of Position, Scale and Rotation.
// It also takes care of the model matrix
class Transform {
	public:
		void set_position(const vec3<float>& position) noexcept;
		void set_scale(const vec3<float>& scale) noexcept;
		void set_rotation(const vec3<float>& rotation) noexcept;

		// -- const access

		inline const vec3<float>& position() const noexcept { return _position; }
		inline const vec3<float>& scale() const noexcept { return _scale; }
		inline const glm::quat& rotation() const noexcept { return _rotation; }
		// Retrives model matrix
		const glm::mat4& model_matrix() const noexcept;
	private:
		glm::quat   _rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
		vec3<float> _position = { 0.0f, 0.0f, 0.0f };
		vec3<float> _scale    = { 1.0f, 1.0f, 1.0f };

		// NOTE: marked as 'mutable' so the get method can be used on const methods
		mutable glm::mat4 cached_matrix;
		mutable bool dirty = true;
};

} // namespace floyd
