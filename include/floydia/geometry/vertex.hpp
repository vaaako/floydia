#pragma once

#include <floydia/types.hpp>

#include <floydia/utilities/hash.hpp>

namespace floyd {

struct Vertex2D {
	vec3<float> pos;
	vec2<float> uv;

	inline bool operator==(const Vertex2D& other) const noexcept {
		return this->pos == other.pos
			&& this->uv == other.uv;
	}
};

struct Vertex {
	vec3<float> pos;
	vec3<float> normal;
	vec2<float> uv;

	inline bool operator==(const Vertex& other) const noexcept {
		return this->pos == other.pos
			&& this->normal == other.normal
			&& this->uv == other.uv;
	}
};


} // namespace floyd

namespace std {
	template <>
	struct hash<floyd::Vertex> {
		size_t operator()(const floyd::Vertex& vertex) const {
			size_t seed = 0;
			floyd::hash::combine(seed, vertex.pos.x);
			floyd::hash::combine(seed, vertex.pos.y);
			floyd::hash::combine(seed, vertex.pos.z);

			floyd::hash::combine(seed, vertex.normal.x);
			floyd::hash::combine(seed, vertex.normal.y);
			floyd::hash::combine(seed, vertex.normal.z);

			floyd::hash::combine(seed, vertex.uv.x);
			floyd::hash::combine(seed, vertex.uv.y);

			return seed;
		}
	};

	template <>
	struct hash<floyd::Vertex2D> {
		size_t operator()(const floyd::Vertex2D& vertex) const {
			size_t seed = 0;
			floyd::hash::combine(seed, vertex.pos.x);
			floyd::hash::combine(seed, vertex.pos.y);
			floyd::hash::combine(seed, vertex.pos.z);

			floyd::hash::combine(seed, vertex.uv.x);
			floyd::hash::combine(seed, vertex.uv.y);

			return seed;
		}
	};
}

