#include "floydia/physics/ray.hpp"
#include <algorithm>
namespace floyd {

float Ray::test_aabb(const AABB& aabb) const noexcept {
	const vec3<float> inv_dir = 1.0f / this->direction;

	const vec3<float> t_min = (aabb.min - this->origin) * inv_dir;
	const vec3<float> t_max = (aabb.max - this->origin) * inv_dir;

	const vec3<float> t_enter = glm::min(t_min, t_max);
	const vec3<float> t_exit  = glm::max(t_min, t_max);

	const float t_near = std::max({ t_enter.x, t_enter.y, t_enter.z });
	const float t_far  = std::min({ t_exit.x, t_exit.y, t_exit.z });

	if(t_far < 0.0f || t_near > t_far) return -1.0f;
	return (t_near >= 0.0f) ? t_near : t_far;
}

Ray Ray::screen_to_ray(const PerspectiveCamera& camera, const vec2<u32>& mouse_pos, const vec2<u32>& win_size) noexcept {
	// Mouse to NDC [-1, 1]
	const vec2<float> ndc = {
		((float)mouse_pos.x / win_size.x) * 2.0f - 1.0f,
		-(((float)mouse_pos.y / win_size.y) * 2.0f - 1.0f) // Inverted Y
	};
	// OpenGL have origin centered, Y grows up. If not inverted, ray will point to opposite side

	const glm::mat4 inv_proj = glm::inverse(camera.projection());
	const glm::mat4 inv_view = glm::inverse(camera.view());
	
	// Clip space to view space
	const vec4<float> ray_clip = { ndc.x, ndc.y, -1.0f, 1.0f };
	vec4<float> ray_view = inv_proj * ray_clip;
	ray_view = { ray_view.x, ray_view.y, -1.0f, 0.0f }; // Direction

	// View space to world space
	const vec3<float> ray_world = glm::normalize(vec3<float>(inv_view * ray_view));
	// Origin = camera position
	const vec3<float> origin = camera.position;

	return { origin, ray_world };
}

} // namespace floyd
