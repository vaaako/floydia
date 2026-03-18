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

out vec2 texuv;
out vec4 color;

layout(std140, binding = 0) uniform CameraBlock {
	mat4 view;
	mat4 proj;
};

struct Instance {
	mat4 model;
	vec4 color;
};

layout(std430, binding = 1) buffer InstanceBuffer {
	Instance instances[];
};

void main() {
	// gl_BaseInstance: Start index
	Instance inst = instances[gl_InstanceID + gl_BaseInstance];
	vec4 worldpos     = inst.model * vec4(aPos, 1.0);
	gl_Position       = proj * view * worldpos;

	texuv = aTex;
	color = inst.color;
}
)glsl";

constexpr const char* DEFAULT_VERTEX_2D = R"glsl(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;

out vec2 texuv;
out vec4 color;

layout(std140, binding = 0) uniform CameraBlock {
	mat4 view;
	mat4 proj;
};

struct Instance {
	mat4 model;
	vec4 color;
};

layout(std430, binding = 1) buffer InstanceBuffer {
	Instance instances[];
};

void main() {
	// gl_BaseInstance: Start index
	Instance inst = instances[gl_InstanceID + gl_BaseInstance];
	vec4 worldpos     = inst.model * vec4(aPos, 1.0);
	gl_Position       = proj * view * worldpos;

	texuv = aTex;
	color = inst.color;
}
)glsl";

constexpr const char* DEFAULT_FRAGMENT = R"glsl(
#version 450 core

in vec2 texuv;
in vec4 color;
out vec4 fragcolor;

// uniform sampler2D tex2d;

void main() {
	fragcolor = color; // * texture(tex2d, texuv);
}
)glsl";

} // namespace Shaders
} // namespace floyd
