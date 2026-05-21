#pragma once

#include <floydia/geometry/vertex.hpp>
#include <floydia/types.hpp>
#include <limits>

namespace floyd {

struct AABB {
	AABB() = default;
	AABB(const std::vector<Vertex>& vertices) noexcept;
	AABB(const std::vector<Vertex2D>& vertices) noexcept;

	vec3<float> min = vec3<float>(std::numeric_limits<float>::max());
	vec3<float> max = vec3<float>(-std::numeric_limits<float>::max());
	bool valid = false;
	bool is_2d = false;

	// Compute local position for AABB
	void compute_local(const std::vector<Vertex>& vertices) noexcept;
	// Compute local position for AABB
	void compute_local(const std::vector<Vertex2D>& vertices) noexcept;
	// Call this when the model matrix changes
	AABB to_world(const glm::mat4& model) const noexcept;

	// Merge another AABB into this one.
	// Used by 'Model' to wrap all submeshes
	void merge(const AABB& other) noexcept;

	// 3D test
	inline bool intersects(const AABB& other) const {
		return (this->min.x <= other.max.x && this->max.x >= other.min.x) &&
			   (this->min.y <= other.max.y && this->max.y >= other.min.y) &&
			   (this->min.z <= other.max.z && this->max.z >= other.min.z);
	}

	// 2D test. Ignores Z
	inline bool intersects_2d(const AABB& other) const {
		return (this->min.x <= other.max.x && this->max.x >= other.min.x) &&
			   (this->min.y <= other.max.y && this->max.y >= other.min.y);
	}


	// Midpoint of box
	inline vec3<float> center() const noexcept { return (this->min + this->max) * 0.5f; }
	// Half size of box
	inline vec3<float> extents() const noexcept { return (this->max - this->min) * 0.5f; }
};

} // namespace floyd
