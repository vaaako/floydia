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

		// Color on 0-255 range
		inline const vec4<float> color() const {
			return (vec4<float>)this->_color * glm::vec4(255.0f);
		}

		inline void set_color(const vec4<uint8>& c) {
			this->_color = (vec4<float>)c / glm::vec4(255.0f);
		}

		// Normalized color
		inline const vec4<float>& color_norm() const {
			return this->_color;
		}

	private:
		// Color is on object because it isn't considered when batching
		vec4<float> _color = vec4<float>(1.0f);

};

} // namespace floyd
