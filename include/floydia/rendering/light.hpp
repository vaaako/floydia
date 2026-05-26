#pragma once

#include "floydia/rendering/object.hpp"
#include "floydia/rendering/script.hpp"
#include <memory>
#include <vector>

namespace floyd {

class Light : public Object {
	public:
		enum Type : u32 {
			// Infinite distance, no position or attenuation
			Directional = 0,
			// Point of light
			Point = 1,
			// Point of light with a cone restriction
			Spot = 2
		};

		struct alignas(16) LightBuffer {
			u32 count;
			u32 _pad[3]; // Remove padding warning
		};

		struct alignas(16) LightData {
			vec4<float> position; // w=0 if directionl
			vec4<float> direction; // w=0 if point
			vec4<float> color; // w=intensity
			float range;
			float inner_angle; // internal cos angle (spot)
			float outer_angle; // external cos angle (spot)
			u32 type; // u32 for compability
		};

	public:
		float intensity = 1.0f;
		float range = 15.0f;
		// Spot only
		float inner_angle = 12.5f; // degrees
		float outer_angle = 17.5f; // degrees
		Light::Type type = Light::Directional;

	public:
		explicit Light(const Light::Type type) noexcept : type(type) {}
		~Light() noexcept = default;

		inline LightData to_gpu_data() const noexcept {
			return {
				.position = vec4<float>(this->transform.position(), (this->type == Light::Directional) ? 0.0f : 1.0f),
				.direction = vec4<float>(this->transform.forward(), 0.0f),
				.color =  vec4<float>(vec3<float>(this->color_norm()), this->intensity),
				.range = range,
				.inner_angle = glm::cos(glm::radians(this->inner_angle)),
				.outer_angle = glm::cos(glm::radians(this->outer_angle)),
				.type = type
			};
		}

	private:
		std::vector<std::unique_ptr<Script>> scripts;
};

}
