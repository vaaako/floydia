#pragma once

// std140: Default layout for UBO
//   + Forced to 16 bytes
// std430: Default layout for SSBO
//   + Not forced to 16 bytes

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

struct InstanceData {
	mat4 model;
	vec4 color;
	// float metallic = 0.0f;
	// float roughness = 1.0f;
	// float _pad[2];
};

layout(std140, binding = 0) uniform CameraBlock {
	mat4 view;
	mat4 proj;
	vec4 campos;
};

layout(std430, binding = 1) buffer InstanceBuffer {
	InstanceData instances[];
};

void main() {
	// gl_BaseInstance: Offset. Start of data
	InstanceData data = instances[gl_InstanceID + gl_BaseInstance];

	vec4 worldpos = data.model * vec4(aPos, 1.0);

	texuv = aTex;
	// normal = mat3(transpose(inverse(data.model))) * aNor; // Grants that normal keep perpendicular after any transformation
	normal = mat3(data.model) * aNor; // Cheaper, but mais be incorrect with non-uniform scale
	color = data.color;
	fragpos = worldpos.xyz;

	gl_Position = proj * view * worldpos;
}
)glsl";

constexpr const char* DEFAULT_FRAGMENT = R"glsl(
#version 460 core

layout(location = 0) in vec2 texuv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec3 fragpos;

layout(location = 0) uniform float u_metallic;
layout(location = 1) uniform float u_roughness;
layout(location = 2) uniform sampler2D albedo;

out vec4 fragcolor;

struct LightData {
	vec4 position; // w=0 if directional
	vec4 direction;
	vec4 color; // a=intensity
	float range;
	float inner_angle; // precomputed cos of inner cone angle (spot)
	float outer_angle; // precomputed cos of outer cone angle (spot)
	uint type;
};

// NOTE: Binding on both shaders do not consume extra space
layout(std140, binding = 0) uniform CameraBlock {
	mat4 view;
	mat4 proj;
	vec4 campos;
};

layout(std430, binding = 2) buffer LightBuffer {
	uint lights_count;
	uint _pad[3]; // align to 16 bytes offset
	LightData lights[];
};

// https://www.mbsoftworks.sk/tutorials/opengl3/17-spotlight/

vec3 calc_light(vec3 n, vec3 light_color, float intensity, vec3 light_dir,
	vec3 view_dir, vec4 base_color, float attenuation) {
	// Blinn-Phong halfway vector
	vec3 halfway = normalize(light_dir + view_dir);
	float diff = max(dot(n, light_dir), 0.0);

	// Roughness
	float shininess = mix(256.0, 1.0, u_roughness);
	float spec      = pow(max(dot(n, halfway), 0.0), shininess);
	// Metallic
	vec3 spec_color = mix(vec3(1.0), base_color.rgb, u_metallic);
	float spec_strength = mix(0.3, 1.0, u_metallic);

	// Diffuse: how directly the surface faces the light
	// Specular: highlight where light reflects toward the camera
	vec3 diffuse = light_color * intensity * diff;
	vec3 specular = light_color * intensity * spec * spec_color * spec_strength;
	
	return (diffuse + specular) * attenuation;
}

vec3 calc_point(LightData light, vec3 n, vec3 view_dir, vec4 base_color) {
	vec3 to_light = light.position.xyz - fragpos; // Surface to lightsource
	float dist    = length(to_light);
	if(dist > light.range) return vec3(0.0); // Fragment is outside light range

	vec3 light_dir = normalize(to_light);
	// 1.0 at center, 0.0 at range boundary
	float attenuation = 1.0 - smoothstep(0.0, light.range, dist);
	return calc_light(n, light.color.rgb, light.color.a, light_dir, view_dir, base_color, attenuation);
}

vec3 calc_spot(LightData light, vec3 n, vec3 view_dir, vec4 base_color) {
	vec3  to_light = normalize(fragpos - light.position.xyz);
	// Angle between fragment direction and spot cone axis
	float theta    = dot(light.direction.xyz, to_light);
	if(theta < light.outer_angle) return vec3(0.0); // Outside the outer cone

	// Smooth falloff between inner and outer cone edges
	float epsilon = light.inner_angle - light.outer_angle;
	float factor  = clamp((theta - light.outer_angle) / epsilon, 0.0, 1.0);
	// Reuse point light calculation, scaled by cone factor
	return calc_point(light, n, view_dir, base_color) * factor;
}

void main() {
	// vec4 base = texture(albedo, texuv) * color;
	// fragcolor = base;

	// vec3 n = normalize(normal) * 0.5 + 0.5;
	// fragcolor = vec4(n, 1.0);

	vec3 n = normalize(normal); // Renormalize after interpolation
	vec3 view_dir = normalize(campos.xyz - fragpos);
	vec4 base = texture(albedo, texuv) * color;

	// Metals absorve more light, the ambient must be darker
	// vec3 ambient = vec3(0.1);
	vec3 ambient = mix(vec3(0.1), base.rgb * 0.05, u_metallic);
	vec3 lighting = ambient;

	// Blinn-Phong specular
	for(uint i = 0; i < lights_count; ++i) {
		LightData light = lights[i];
		// Directional
		switch(light.type) {
			// Directional
			case 0u: {
				vec3 light_dir = normalize(-light.direction.xyz);
				float attenuation = 1.0; // Directional has no attenuation
				lighting += calc_light(n, light.color.rgb, light.color.a, light_dir, view_dir, base, attenuation);
				break;
			}
			case 1u: lighting += calc_point(light, n, view_dir, base); break;
			case 2u: lighting += calc_spot(light, n, view_dir, base); break;
			default: break;
		}
	}

	// float dist = length(lights[0].position.xyz - fragpos);
	// fragcolor = vec4(dist / 50.0, 0.0, 0.0, 1.0);
	// fragcolor = vec4(fragpos * 0.1 + 0.5, 1.0);
	
	fragcolor = vec4(base.rgb * lighting, base.a);
}
)glsl";



constexpr const char* DEFAULT_VERTEX_2D = R"glsl(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;

layout(location = 0) out vec2 texuv;
layout(location = 1) out vec4 color;

out gl_PerVertex {
	vec4 gl_Position;
};

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



constexpr const char* DEFAULT_FRAGMENT_TEXT = R"glsl(
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


// No light 3D object
constexpr const char* DEFAULT_FRAGMENT_UNLIT = R"glsl(
#version 460 core

layout(location = 0) in vec2 texuv;
layout(location = 1) in vec3 normal;  // Not used but have to declare
layout(location = 2) in vec4 color;
layout(location = 3) in vec3 fragpos;// Not used but have to declare 

out vec4 fragcolor;
uniform sampler2D albedo;

void main() {
	vec4 base = texture(albedo, texuv) * color;
	fragcolor = base;
}
)glsl";

} // namespace Shaders
} // namespace floyd
