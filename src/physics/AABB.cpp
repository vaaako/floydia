#include <floydia/physics/AABB.hpp>

namespace floyd {

void AABB::compute_local_aabb(const std::vector<Vertex>& vertices) noexcept {
	if(vertices.empty()) return;
	for(const Vertex& v : vertices) {
		this->min = glm::min(this->min, v.pos);
		this->max = glm::max(this->max, v.pos);
	}
	this->valid = true;
}

AABB AABB::to_world(const glm::mat4& model) const noexcept {
	if(!this->valid) return {};
	// Transform all 8 corners, re-fit a new AABB around them
	// THis is correct even with non-uniform scale or rotation
	const vec3<float> corners[8] = {
		{min.x, min.y, min.z}, {max.x, min.y, min.z},
		{min.x, max.y, min.z}, {max.x, max.y, min.z},
		{min.x, min.y, max.z}, {max.x, min.y, max.z},
		{min.x, max.y, max.z}, {max.x, max.y, max.z}
	};

	AABB world;
	world.min = vec3<float>(std::numeric_limits<float>::max());
	world.max = vec3<float>(-std::numeric_limits<float>::max());
	for(const vec3<float>& c : corners) {
		const vec3<float> wc = vec3<float>(model * vec4<float>(c, 1.0f));
		world.min = glm::min(world.min, wc);
		world.max = glm::max(world.max, wc);
	}
	world.valid = true;
	return world;
}

void AABB::merge(const AABB& other) noexcept {
	if(!other.valid) return;
	this->min = glm::min(this->min, other.min);
	this->max = glm::max(this->max, other.max);
	this->valid = true;
}

} // namespace floyd
