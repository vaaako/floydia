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

// https://www.mbsoftworks.sk/tutorials/opengl3/17-spotlight/
constexpr const char* DEFAULT_VERTEX = R"glsl(
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNor;
layout (location = 2) in vec2 aTex;

layout (location = 0) out vec2 texuv;
layout (location = 1) out vec4 lit_color; // color * lightining, computed per-vertex
layout (location = 2) out float fog_factor;

// Which built-ins this stage uses
// Required for Program Pipeline
out gl_PerVertex {
	vec4 gl_Position;
};

struct InstanceData {
	mat4 model;
	vec4 color;
	float metallic;
	float roughness;
	vec2 _pad;
};

struct LightData {
	vec4 position;
	vec4 direction;
	vec4 color;
	float range;
	float inner_angle;
	float outer_angle;
	uint type;
};

layout(std140, binding = 0) uniform CameraBlock {
	mat4 view;
	mat4 proj;
	vec4 campos;
};

layout(std430, binding = 1) buffer InstanceBuffer {
	InstanceData instances[];
};

layout(std430, binding = 2) buffer LightBuffer {
	uint lights_count;
	uint _pad[3];
	LightData lights[];
};

const float DEFAULT_FOG_START = 20.0;
const float DEFAULT_FOG_END = 100.0;

layout (location = 1) uniform float u_ambient;
layout (location = 2) uniform float u_ambient_factor; // 0.0 = disabled (metals use flat ambient)
layout (location = 3) uniform float u_fog_start;
layout (location = 4) uniform float u_fog_end;
layout (location = 5) uniform float u_fog_disabled; // 1.0 = disable fog

vec3 calc_light(vec3 n, vec3 light_color, float intensity, vec3 light_dir,
	vec3 view_dir, vec3 base_color, float metallic, float roughness, float attenuation) {
	// Blinn-Phong halfway vector
	vec3 halfway = normalize(light_dir + view_dir);
	float diff = max(dot(n, light_dir), 0.0);

	// Roughness
	float shininess = mix(256.0, 1.0, roughness);
	float spec      = pow(max(dot(n, halfway), 0.0), shininess);
	vec3 spec_color = mix(vec3(1.0), base_color, metallic);
	// Metallic
	float spec_strength = mix(0.3, 1.0, metallic);

	// Diffuse: how directly the surface faces the light
	// Specular: highlight where light reflects toward the camera
	vec3 diffuse = light_color * intensity * diff;
	vec3 specular = light_color * intensity * spec * spec_color * spec_strength;
	// vec3 specular = light_color * intensity * spec * spec_strength;
	// NOTE: 'spec_color' was dropped here. It dependedd on the fragment's texture sample
	// which is not available here
	
	return (diffuse + specular) * attenuation;
}

vec3 calc_point(LightData light, vec3 n, vec3 view_dir, vec3 fragpos, vec3 base_color, float metallic, float roughness) {
	vec3 to_light = light.position.xyz - fragpos; // Surface to lightsource
	float dist = length(to_light);
	if(dist > light.range) return vec3(0.0); // Fragment is outside light range

	vec3 light_dir = normalize(to_light);
	// 1.0 at center, 0.0 at range boundary
	float attenuation = 1.0 - smoothstep(0.0, light.range, dist);
	return calc_light(n, light.color.rgb, light.color.a, light_dir, view_dir, base_color, metallic, roughness, attenuation);
}

vec3 calc_spot(LightData light, vec3 n, vec3 view_dir, vec3 fragpos, vec3 base_color, float metallic, float roughness) {
	vec3 to_light = normalize(fragpos - light.position.xyz);
	// Angle between fragment direction and spot cone axis
	float theta = dot(light.direction.xyz, to_light);
	if(theta < light.outer_angle) return vec3(0.0); // Outside the outer cone

	// Smooth falloff between inner and outer cone edges
	float epsilon = light.inner_angle - light.outer_angle;
	float factor  = clamp((theta - light.outer_angle) / epsilon, 0.0, 1.0);
	// Reuse point light calculation, scaled by cone factor
	return calc_point(light, n, view_dir, fragpos, base_color, metallic, roughness) * factor;
}


void main() {
	// gl_BaseInstance: Offset. Start of data
	InstanceData data = instances[gl_InstanceID + gl_BaseInstance];

	vec4 worldpos = data.model * vec4(aPos, 1.0);
	vec3 n = normalize(mat3(data.model) * aNor);
	vec3 view_dir = normalize(campos.xyz - worldpos.xyz);

	// Ambient lighting reaching this surface
	// - 'u_ambient_factor' <= 0.0: metals get the same flat ambient as non-metals
	//    Useful for scenes without a rich ambient/reflection source to make metal look right
	// - 'u_ambient_factor' > 0.0: metals are darkened toward,
	//    non-metals stay at full 'u_ambient'. Closer to physically-based behaviour
	//    (metals have no diffuse response, and only reflect their surroundings)
	//    at the cost of metal objects going nearly black in areas with no direct light
	vec3 ambient = vec3(u_ambient);
	if(u_ambient_factor > 0.0) ambient = mix(vec3(u_ambient), vec3(u_ambient * u_ambient_factor), data.metallic);
	vec3 lighting = ambient;

	for(uint i = 0; i < lights_count; ++i) {
		LightData light = lights[i];
		switch(light.type) {
			// Directional
			case 0u: {
				vec3 light_dir = normalize(-light.direction.xyz);
				float attenuation = 1.0; // Directional has no attenuation
				lighting += calc_light(n, light.color.rgb, light.color.a, light_dir, view_dir,
					data.color.rgb, data.metallic, data.roughness, attenuation);
				break;
			}
			case 1u: lighting += calc_point(light, n, view_dir, worldpos.xyz,
										data.color.rgb, data.metallic, data.roughness); break;
			case 2u: lighting += calc_spot(light, n, view_dir, worldpos.xyz,
										data.color.rgb, data.metallic, data.roughness); break;
			default: break;
		}
	}

	float fog_start = (u_fog_start == 0.0) ? DEFAULT_FOG_START : u_fog_start;
	float fog_end   = (u_fog_end == 0.0) ? DEFAULT_FOG_END   : u_fog_end;

	float dist = length(campos.xyz - worldpos.xyz);
	fog_factor = (u_fog_disabled < 0.5)
		? clamp((dist - fog_start) / (fog_end - fog_start), 0.0, 1.0)
		: 0.0;

	texuv = aTex;
	lit_color = vec4(data.color.rgb * lighting, data.color.a);
	gl_Position = proj * view * worldpos;
}
)glsl";



constexpr const char* DEFAULT_FRAGMENT = R"glsl(
#version 460 core

layout (location = 0) in vec2 texuv;
layout (location = 1) in vec4 lit_color;
layout (location = 2) in float fog_factor;

layout (location = 0) uniform sampler2D albedo;
layout (location = 1) uniform vec3 u_fog_color;

layout (location = 0) out vec4 fragcolor;

void main() {
	// fragcolor = texture(albedo, texuv) * lit_color;
	vec4 base = texture(albedo, texuv) * lit_color;
	fragcolor = vec4(mix(base.rgb, u_fog_color, fog_factor), base.a);
}
)glsl";



constexpr const char* DEFAULT_BILLBOARD_VERTEX = R"glsl(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNor;
layout(location = 2) in vec2 aTex;

layout(location = 0) out vec2 texuv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec4 color;
layout(location = 3) out vec3 fragpos;
layout(location = 4) flat out float v_metallic;
layout(location = 5) flat out float v_roughness;

out gl_PerVertex {
	vec4 gl_Position;
};

struct InstanceData {
	mat4 model;
	vec4 color;
	float metallic;
	float roughness;
	float billboard_type; // 0 = Full, 1 = Cylindrical
	float _pad;
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
	InstanceData data = instances[gl_InstanceID + gl_BaseInstance];

	// Extract scale from model matrix columns
	const float sx = length(data.model[0].xyz);
	const float sy = length(data.model[1].xyz);
	const float sz = length(data.model[2].xyz);

	// World position from model matrix translation column
	const vec3 world_pos = data.model[3].xyz;

	// Extract view right/up/forward from view matrix rows
	// View matrix rows are the world-space camera axis
	const vec3 right   = vec3(view[0][0], view[1][0], view[2][0]);
	const vec3 up      = vec3(view[0][1], view[1][1], view[2][1]);
	const vec3 forward = vec3(view[0][2], view[1][2], view[2][2]);

	vec3 v_right;
	vec3 v_up;
	vec3 v_forward;

	if(data.billboard_type == 0.0) {
		// All axis face camera
		v_right   = right   * sx;
		v_up      = up      * sy;
		v_forward = forward * sz;
	} else {
		// Only X and Z face camera
		v_right   = right * sx;
		v_up      = data.model[1].xyz;
		v_forward = forward * sz;
	}

	// Reconstruct model matrix with billboard axis
	mat4 billboard = mat4(
		vec4(v_right,   0.0),
		vec4(v_up,      0.0),
		vec4(v_forward, 0.0),
		vec4(world_pos, 1.0)
	);

	vec4 worldpos = billboard * vec4(aPos, 1.0);
	texuv   = aTex;
	normal  = mat3(billboard) * aNor;
	color   = data.color;
	fragpos = worldpos.xyz;
	v_metallic = data.metallic;
	v_roughness = data.roughness;

	gl_Position = proj * view * worldpos;
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
	float metallic;
	float roughness;
	vec2 _pad;
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

layout(location = 0) out vec4 fragcolor;

layout(location = 0) uniform sampler2D albedo;

void main() {
	vec4 base = texture(albedo, texuv) * color;
	fragcolor = base;
}
)glsl";




constexpr const char* DEFAULT_VERTEX_TEXT = R"glsl(
#version 460 core

layout(location = 0) out vec2 texuv;
layout(location = 1) out vec4 color;

out gl_PerVertex {
	vec4 gl_Position;
};

struct GlyphData {
	vec2 pos;
	vec2 size;
	vec2 uv0;
	vec2 uv1;
	vec4 color;
};

layout(std430, binding = 3) buffer GlyphBuffer {
	GlyphData glyphs[];
};

layout(location = 0) uniform vec2 u_screen_size;

const vec2 CORNERS[6] = vec2[6](
	vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
	vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
);

void main() {
	GlyphData g = glyphs[gl_InstanceID + gl_BaseInstance];
	vec2 corner = CORNERS[gl_VertexID];

	vec2 px  = g.pos + corner * g.size;
	vec2 ndc = (px / u_screen_size) * 2.0 - 1.0;
	ndc.y    = -ndc.y; // flip y: screen top-left

	gl_Position = vec4(ndc, 0.0, 1.0);
	texuv = g.uv0 + corner * (g.uv1 - g.uv0);
	color = g.color;
}
)glsl";


constexpr const char* DEFAULT_AABB_VERTEX = R"glsl(
#version 460 core
layout(location = 0) uniform mat4 u_viewproj;
layout(location = 1) uniform vec3 u_min;
layout(location = 2) uniform vec3 u_max;

out gl_PerVertex { vec4 gl_Position; };

const vec3 corners[8] = vec3[8](
	vec3(0,0,0), vec3(1,0,0), vec3(1,1,0), vec3(0,1,0),
	vec3(0,0,1), vec3(1,0,1), vec3(1,1,1), vec3(0,1,1)
);

const int lines[24] = int[24](
	0,1, 1,2, 2,3, 3,0,
	4,5, 5,6, 6,7, 7,4,
	0,4, 1,5, 2,6, 3,7
);

void main() {
	vec3 c = mix(u_min, u_max, corners[lines[gl_VertexID]]);
	gl_Position = u_viewproj * vec4(c, 1.0);
}
)glsl";

constexpr const char* DEFAULT_AABB_FRAGMENT = R"glsl(
#version 460 core

layout(location = 3) uniform vec3 u_color;
layout(location = 0) out vec4 fragcolor;

void main() {
	fragcolor = vec4(u_color, 1.0);
}
)glsl";

// No light 3D object
constexpr const char* DEFAULT_FRAGMENT_UNLIT = R"glsl(
#version 460 core

layout(location = 0) in vec2 texuv;
layout(location = 1) in vec3 normal; // unused
layout(location = 2) in vec4 color;
layout(location = 3) in vec3 fragpos; // unused

layout(location = 0) uniform sampler2D albedo;
layout(location = 0) out vec4 fragcolor;

void main() {
	vec4 base = texture(albedo, texuv) * color;
	fragcolor = base;
}
)glsl";


} // namespace Shaders
} // namespace floyd
