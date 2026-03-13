#pragma once

#include <floydia/types.hpp>
#include <floydia/utilities/hash.hpp>

struct Vertex {
	vec3<float> position;
	vec2<float> texuv;

	bool operator==(const Vertex& other) const noexcept {
		return glm::all(glm::epsilonEqual(this->position, other.position, 0.0001f))
			&& glm::all(glm::epsilonEqual(this->texuv, other.texuv, 0.0001f));
	}
}

namespace std {
	template <typename T>
	struct hash<Vertex> {
		size_t operator()(const Vertex& vertex) const {
			size_t seed = 0;
			hash::combine(seed, vertex.position.x);
			hash::combine(seed, vertex.position.y);
			hash::combine(seed, vertex.position.z);

			hash::combine(seed, vertex.texuv.x);
			hash::combine(seed, vertex.texuv.y);

			return seed;
		}
	}
}

