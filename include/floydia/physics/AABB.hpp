#pragma once

#include <floydia/geometry/vertex.hpp>
#include <floydia/types.hpp>
#include <limits>

namespace floyd {

struct AABB {
	AABB() = default;

	vec3<float> min = vec3<float>(std::numeric_limits<float>::max());
	vec3<float> max = vec3<float>(-std::numeric_limits<float>::max());
	bool valid = false;

	// Compute local position for AABB
	void compute_local_aabb(const std::vector<Vertex>& vertices) noexcept;
	// Call this when the model matrix changes
	AABB to_world(const glm::mat4& model) const noexcept;

	// Merge another AABB into this one.
	// Used by 'Model' to wrap all submeshes
	inline void merge(const AABB& other) {
		if(!other.valid) return;
		this->min = glm::min(this->min, other.min);
		this->max = glm::max(this->max, other.max);
		this->valid = true;
	}

	// Midpoint of box
	inline vec3<float> center() const noexcept {
		return (this->min + this->max) * 0.5f;
	}

	// Half size of box
	inline vec3<float> extents() const noexcept {
		return (this->max - this->min) * 0.5f;
	}
};

} // namespace floyd
