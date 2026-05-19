#pragma once

#include "floydia/camera/perspectivecamera.hpp"
#include "floydia/physics/AABB.hpp"
#include "floydia/types.hpp"

namespace floyd {

struct Ray {
	vec3<float> origin;
	vec3<float> direction; // Normalized

	// Returns -1 if no hit
	float test_aabb(const AABB& aabb) const noexcept;

	// Converts mouse position to a world-space ray
	static Ray screen_to_ray(const PerspectiveCamera& camera, const vec2<u32>& mouse_pos, const vec2<u32>& win_size) noexcept;
};

} // namespace floyd
