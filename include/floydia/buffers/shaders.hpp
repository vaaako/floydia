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

const char* DEFAULT_VERTEX = R"glsl(
#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;

out vec2 texuv;

layout(std140, binding = 0) uniform CameraBlock {
	mat4 view;
	mat4 proj;
};

layout(std430, binding = 1) buffer InstanceBuffer {
	mat4 models[];
};

void main() {
	mat4 model_matrix = model[gl_InstanceID];
	vec4 worldpos     = model_matrix * vec4(aPos, 1.0);
	gl_Position       = proj * view * worldpos;
	texuv = aTex;
}
)glsl";

const char* DEFAULT_FRAGMENT = R"glsl(
#version 450 core

in vec2 texuv;
out vec4 fragcolor

uniform vec4 color;
uniform sampler2D tex2d;

void main() {
	fragcolor = color * texture(tex2d, texuv);
}
)glsl";

} // namespace Shaders
} // namespace floyd
