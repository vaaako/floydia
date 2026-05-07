#pragma once

#include <floydia/rendering/transform.hpp>


namespace floyd {

// Just using as reference, so this is ok
class Renderer;

// Transformable object
class Object {
	public:
		Transform transform;

	public:
		Object() = default;
		virtual ~Object() = default;

		inline void set_color(const vec4<uint8>& c) {
			this->color = (vec4<float>)c / glm::vec4(255.0f);
		}

		// Normalized color
		inline const vec4<float>& color_norm() const {
			return this->color;
		}

	private:
		// Color is here so that batch draw
		// can continue working regardless of the object's color
		vec4<float> color = vec4<float>(1.0f);
		// float metallic = 0.0f;
		// float roughness = 1.0f;

};

} // namespace floyd
