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
layout(location = 1) out vec4 color;

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

constexpr const char* DEFAULT_FRAGMENT = R"glsl(
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
