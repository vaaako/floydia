#pragma once

// std140 -> Default layout for UBO
// std430 -> Default layout for SSBO

// UBO is used for small data
// SSBO is used for big data

/*
UBO
	Camera
	Frame Data

SSBO
	Instance transforms
	Material indices
	Light lists
*/


namespace floyd {
namespace Shaders {

constexpr const char* DEFAULT_VERTEX = R"glsl(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNor;
layout(location = 2) in vec2 aTex;

layout(location = 0) out vec2 texuv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec4 color;
layout(location = 3) out vec3 fragpos;

// Which built-ins this stage uses
// Required for Program Pipeline
out gl_PerVertex {
	vec4 gl_Position;
};


// For objects
struct InstanceData {
	mat4 model;
	vec4 color;
};

layout(std140, binding = 0) uniform CameraBlock {
	mat4 view;
	mat4 proj;
};

layout(std430, binding = 1) buffer InstanceBuffer {
	InstanceData instances[];
};

void main() {
	// gl_BaseInstance: Offset. Start of data
	InstanceData data = instances[gl_InstanceID + gl_BaseInstance];

	vec4 worldpos = data.model * vec4(aPos, 1.0);

	texuv = aTex;
	normal = mat3(transpose(inverse(data.model))) * aNor; // Grants that normal keep perpendicular after any transformation
	// normal = mat3(data.model) * aNor; // Cheaper, but mais be incorrect with non-uniform scale
	color = data.color;
	fragpos = worldpos.xyz;
	gl_Position  = proj * view * worldpos;
}
)glsl";

constexpr const char* DEFAULT_FRAGMENT = R"glsl(
#version 460 core

layout(location = 0) in vec2 texuv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec3 fragpos;

out vec4 fragcolor;
uniform sampler2D albedo;

// For light
struct LightData {
	vec4 position; // w=0 if directional
	vec4 direction;
	vec4 color; // w=intensity
	float range;
	float inner_range;
	float outer_angle;
	uint type;
};

layout(std430, binding = 2) buffer LightBuffer {
	uint light_count;
	LightData lights[];
};


void main() {
	// vec3 n = normalize(normal) * 0.5 + 0.5;
	// fragcolor = vec4(n, 1.0);
	vec3 n = normalize(normal) * 0.5 + 0.5;
	vec3 ambient = vec3(0.1);
	vec3 lighting = ambient;

	for(uint i = 0; i < light_count; ++i) {
		LightData light = lights[i];
		// Directional
		if(light.type == 0u) {
			vec3 light_dir = normalize(-light.direction.xyz);
			float diff = max(dot(n, light_dir), 0.0);
			lighting += light.color.rgb * light.color.a * diff;
		}
	}

	vec4 base = texture(albedo, texuv) * color;
	fragcolor = vec4(base.rgb * lighting, base.a);
}
)glsl";



constexpr const char* DEFAULT_VERTEX_2D = R"glsl(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;

layout(location = 0) out vec2 texuv;
layout(location = 1) out vec4 color;

// Which built-ins this stage uses
// Required for Program Pipeline
out gl_PerVertex {
	vec4 gl_Position;
};

layout(std140, binding = 0) uniform CameraBlock {
	mat4 view;
	mat4 proj;
};

struct InstanceData {
	mat4 model;
	vec4 color;
};

layout(std430, binding = 1) buffer InstanceBuffer {
	InstanceData instances[];
};

void main() {
	// gl_BaseInstance: Offset. Start of data
	InstanceData data = instances[gl_InstanceID + gl_BaseInstance];

	vec4 worldpos     = data.model * vec4(aPos, 1.0);
	gl_Position       = proj * view * worldpos;

	texuv = aTex;
	color = data.color;
}
)glsl";

constexpr const char* DEFAULT_FRAGMENT_2D = R"glsl(
#version 460 core

layout(location = 0) in vec2 texuv;
layout(location = 1) in vec4 color;

out vec4 fragcolor;

uniform sampler2D albedo;

void main() {
	vec4 base = texture(albedo, texuv) * color;
	fragcolor = base;
}
)glsl";



constexpr const char* TEXT_FRAGMENT = R"glsl(
#version 460 core

layout(location = 0) in vec2 texuv;
layout(location = 1) in vec4 color;

out vec4 fragcolor;
uniform sampler2D u_atlas;

void main() {
	float alpha = texture(u_atlas, texuv).r; // atlas is single channel
	fragcolor = vec4(color.rgb, color.a * alpha);
}

)glsl";

} // namespace Shaders
} // namespace floyd
