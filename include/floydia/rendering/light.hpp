#pragma once

#include <floydia/rendering/object.hpp>

namespace floyd {

class Light : public Object {
	public:
		enum Type : uint32 {
			Directional = 0,
			Point = 1,
			Spot = 2
		};

		struct alignas(16) LightData {
			vec4<float> position; // w=0 if directionl
			vec4<float> direction; // w=0 if point
			vec4<float> color; // w=intensity
			float range;
			float inner_angle; // internal cos angle (spot)
			float outer_angle; // external cos angle (spot)
			uint32 type; // uint32 for compability
		};

	public:
		float intensity = 1.0f;
		float range = 10.0f;
		// Spot only
		float inner_angle = 12.5f; // degrees
		float outer_angle = 17.5f; // degrees
		Light::Type type = Light::Directional;

	public:
		Light() noexcept = default;
		explicit Light(const Light::Type type) noexcept : type(type) {}
		~Light() noexcept = default;

		inline LightData to_gpu_data() const noexcept {
			return {
				vec4<float>(this->transform.position(), (this->type == Light::Directional) ? 0.0f : 1.0f),
				vec4<float>(this->transform.forward(), 0.0f),
				vec4<float>(vec3<float>(this->color_norm()), this->intensity),
				range,
				glm::cos(glm::radians(this->inner_angle)),
				glm::cos(glm::radians(this->outer_angle)),
				type
			};
		}
};
}
