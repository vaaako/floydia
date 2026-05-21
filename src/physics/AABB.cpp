#include <floydia/physics/AABB.hpp>

namespace floyd {

AABB::AABB(const std::vector<Vertex>& vertices) noexcept {
	this->compute_local(vertices);
}

AABB::AABB(const std::vector<Vertex2D>& vertices) noexcept {
	this->compute_local(vertices);
}

void AABB::compute_local(const std::vector<Vertex>& vertices) noexcept {
	if(vertices.empty()) return;
	for(const Vertex& v : vertices) {
		this->min = glm::min(this->min, v.pos);
		this->max = glm::max(this->max, v.pos);
	}
	this->valid = true;
}

void AABB::compute_local(const std::vector<Vertex2D>& vertices) noexcept {
	if(vertices.empty()) return;
	for(const Vertex2D& v : vertices) {
		this->min = glm::min(this->min, v.pos);
		this->max = glm::max(this->max, v.pos);
	}
	this->min.z = 0.0f;
	this->max.z = 0.0f;
	this->valid = true;
	this->is_2d = true;
}

AABB AABB::to_world(const glm::mat4& model) const noexcept {
	if(!this->valid) return {};

	AABB world;
	world.min = vec3<float>(std::numeric_limits<float>::max());
	world.max = vec3<float>(-std::numeric_limits<float>::max());
	world.valid = true;
	world.is_2d = this->is_2d;

	// Transform all corners, re-fit a new AABB around them
	// This is correct even with non-uniform scale or rotation
	if(this->is_2d) {
		const vec3<float> corners[4] = {
			{ min.x, min.y, 0.0f }, { max.x, min.y, 0.0f },
			{ min.x, max.y, 0.0f }, { max.x, max.y, 0.0f },
		};

		for(const vec3<float>& c : corners) {
			const vec3<float> wc = vec3<float>(model * vec4<float>(c, 1.0f));
			world.min = glm::min(world.min, wc);
			world.max = glm::max(world.max, wc);
		}

		world.min.z = 0.0f;
		world.max.z = 0.0f;

	} else {
		const vec3<float> corners[8] = {
			{min.x, min.y, min.z}, {max.x, min.y, min.z},
			{min.x, max.y, min.z}, {max.x, max.y, min.z},
			{min.x, min.y, max.z}, {max.x, min.y, max.z},
			{min.x, max.y, max.z}, {max.x, max.y, max.z}
		};

		for(const vec3<float>& c : corners) {
			const vec3<float> wc = vec3<float>(model * vec4<float>(c, 1.0f));
			world.min = glm::min(world.min, wc);
			world.max = glm::max(world.max, wc);
		}
	}

	return world;
}

void AABB::merge(const AABB& other) noexcept {
	if(!other.valid) return;
	this->min = glm::min(this->min, other.min);
	this->max = glm::max(this->max, other.max);
	this->valid = true;
}

} // namespace floyd
