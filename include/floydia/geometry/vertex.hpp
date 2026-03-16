#pragma once

#include <floydia/types.hpp>

#include <floydia/utilities/hash.hpp>

namespace floyd {

struct Vertex {
	vec3<float> position;
	vec3<float> normal;
	vec2<float> texuv;

	bool operator==(const Vertex& other) const noexcept {
		constexpr float e = 0.0001f;
		return glm::all(glm::epsilonEqual(this->position, other.position, e))
			&& glm::all(glm::epsilonEqual(this->normal, other.normal, e))
			&& glm::all(glm::epsilonEqual(this->texuv, other.texuv, e));
	}
};

} // namespace floyd

namespace std {
	template <>
	struct hash<floyd::Vertex> {
		size_t operator()(const floyd::Vertex& vertex) const {
			size_t seed = 0;
			floyd::hash::combine(seed, vertex.position.x);
			floyd::hash::combine(seed, vertex.position.y);
			floyd::hash::combine(seed, vertex.position.z);

			floyd::hash::combine(seed, vertex.normal.x);
			floyd::hash::combine(seed, vertex.normal.y);
			floyd::hash::combine(seed, vertex.normal.z);

			floyd::hash::combine(seed, vertex.texuv.x);
			floyd::hash::combine(seed, vertex.texuv.y);

			return seed;
		}
	};
}

