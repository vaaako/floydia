#pragma once

#include <floydia/physics/AABB.hpp>
#include <floydia/types.hpp>

// https://github.com/thecodeofnotch/rd-131655/blob/master/src/main/java/com/mojang/rubydung/level/Frustum.java

namespace floyd {

struct Frustum {
	// left, right, bottom, top, near, far
	vec4<float> planes[6];

	// Build planes from view and projection matrix
	void update(const glm::mat4& vp) noexcept;
	// Test a local-space AABB against the frustum given its world transform
	bool test(const AABB& aabb, const glm::mat4& model) const noexcept;
	// Test a world-space AABB against the frustum
	bool test(const AABB& aabb) const noexcept;
};

} // namespace floyd
