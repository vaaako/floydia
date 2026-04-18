#include <floydia/camera/frustum.hpp>

namespace floyd {

void Frustum::update(const glm::mat4& vp) noexcept {
	const glm::mat4& m = vp; // Frustum

	this->planes[0] = vec4<float>( // left
		m[0][3] + m[0][0],
		m[1][3] + m[1][0],
		m[2][3] + m[2][0],
		m[3][3] + m[3][0]
	);
	this->planes[1] = vec4<float>( // right
		m[0][3] - m[0][0],
		m[1][3] - m[1][0],
		m[2][3] - m[2][0],
		m[3][3] - m[3][0]
	);
	this->planes[2] = vec4<float>( // bottom
		m[0][3] + m[0][1],
		m[1][3] + m[1][1],
		m[2][3] + m[2][1],
		m[3][3] + m[3][1]
	);
	this->planes[3] = vec4<float>( // top
		m[0][3] - m[0][1],
		m[1][3] - m[1][1],
		m[2][3] - m[2][1],
		m[3][3] - m[3][1]
	);
	this->planes[4] = vec4<float>( // near
		m[0][3] + m[0][2],
		m[1][3] + m[1][2],
		m[2][3] + m[2][2],
		m[3][3] + m[3][2]
	);
	// far
	this->planes[5] = vec4<float>(
		m[0][3] - m[0][2],
		m[1][3] - m[1][2],
		m[2][3] - m[2][2],
		m[3][3] - m[3][2]
	);

	// Normalize all planes
	for(vec4<float>& plane : this->planes) {
		const float len = glm::length(vec3<float>(plane));
		plane /= len;
	}
}

bool Frustum::test(const AABB& aabb, const glm::mat4& model) const noexcept {
	if(!aabb.valid) return true; // No AABB data, assume visible
	// Compute world-space AABB by transforming local AABB with model matrix
	// Instead of all 8 corners, use the positive vertex trick:
	// for each plane normal, pick the corner most in that direction
	for(const vec4<float>& plane : this->planes) {
		const vec3<float> normal = vec3<float>(plane);
		// Pick the positive vertex, the corner maximally aligned with the plane normal
		const vec3<float> positive = {
			(normal.x) >= 0.0f ? aabb.max.x : aabb.min.x,
			(normal.y) >= 0.0f ? aabb.max.y : aabb.min.y,
			(normal.z) >= 0.0f ? aabb.max.z : aabb.min.z
		};
		// Transform that single point into world space
		const vec3<float> world = vec3<float>(model * vec4<float>(positive, 1.0f));
		// If the most positive point is behind this plane, the whole AABB is outside
		if(glm::dot(normal, world) + plane.w < 0.0f) return false;
	}
	return true;
}

bool Frustum::test(const AABB& aabb) const noexcept {
	if(!aabb.valid) return true; // No AABB data, assume visible
	for(const vec4<float>& plane : this->planes) {
		const vec3<float> normal = vec3<float>(plane);
		// Pick the positive vertex, the corner maximally aligned with the plane normal
		const vec3<float> positive = {
			(normal.x) >= 0.0f ? aabb.max.x : aabb.min.x,
			(normal.y) >= 0.0f ? aabb.max.y : aabb.min.y,
			(normal.z) >= 0.0f ? aabb.max.z : aabb.min.z
		};
		// If the most positive point is behind this plane, the whole AABB is outside
		if(glm::dot(normal, positive) + plane.w < 0.0f) return false;
	}
	return true;
}

} // namespace floyd
