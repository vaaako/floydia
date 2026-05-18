#pragma once

#include "floydia/rendering/transform.hpp"
#include <atomic>
#include <string>

namespace floyd {

// Transformable object
class Object {
	public:
		Transform transform;
		std::string name = "Object";

	public:
		inline Object() noexcept : _uuid(Object::generate_uuid()) {}
		virtual ~Object() = default;

		// Generate new UUID and copy name
		inline Object(const Object& other) noexcept
			: transform(other.transform), name(other.name), _color(other._color), _uuid(Object::generate_uuid()) {}
		// Dont copy UUID
		Object& operator=(const Object& other) noexcept {
			if(this != &other) {
				this->transform = other.transform;
				this->name = other.name;
				this->_color = other._color;
			}
			return *this;
		}

		// Returns this object UUID
		inline uint64_t uuid() const noexcept { return _uuid; }

		// Color on 0-255 range
		inline const vec4<float> color() const {
			return (vec4<float>)this->_color * glm::vec4(255.0f);
		}

		inline void set_color(const vec4<u8>& c) {
			this->_color = (vec4<float>)c / glm::vec4(255.0f);
		}

		// Normalized color
		inline const vec4<float>& color_norm() const {
			return this->_color;
		}

	private:
		// Color is on object because it isn't considered when batching
		vec4<float> _color = vec4<float>(1.0f);
		u64 _uuid;

		static u64 generate_uuid() noexcept {
			static std::atomic<u64> counter = 1;
			return counter++;
		}

};

} // namespace floyd

