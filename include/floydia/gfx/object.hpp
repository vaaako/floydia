#pragma once

#include <floydia/gfx/transform.hpp>


namespace floyd {

// Just using as reference, so this is ok
class Renderer;

// Transformable object
class Object {
	public:
		Transform transform;
		Object() = default;
		virtual ~Object() = default;
};

} // namespace floyd
