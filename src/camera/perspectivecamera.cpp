#include <floydia/camera/perspectivecamera.hpp>

namespace floyd {

PerspectiveCamera::PerspectiveCamera(const float fov, const float width, const float height) noexcept
	: Camera(width, height), fov(fov) {}

glm::mat4 PerspectiveCamera::projection() const noexcept {
	if(this->dirty) {
		this->proj = glm::perspective(
			glm::radians(this->fov), (this->width / this->height),
			this->near_plane, this->far_plane
		);

		this->dirty = false;
	}
	return this->proj;
}

void PerspectiveCamera::move(const vec3<float>& dir, const float velocity) noexcept {
	// Convert to world position
	vec3<float> world_dir =
		this->forward * (-dir.z) // Invert Z so -Z = forward
		+ this->right * dir.x;
	// Normalize only horizontal (XZ) to prevent faster diagonal movement
	if(glm::length(world_dir) > 0.0f) {
		world_dir = glm::normalize(world_dir);
	}
	// Horizontal + Vertical
	this->position += (world_dir + this->up * dir.y) * velocity;
}


void PerspectiveCamera::rotate(const vec2<float>& delta) noexcept {
	this->yaw += delta.x * this->sensitivity;
	// -Y so UP/DOWN is correct
	this->pitch += -delta.y * this->sensitivity;
	this->update_vectors();
}

void PerspectiveCamera::update_vectors() noexcept {
	// Prevent gimbal flip
	pitch = glm::clamp(this->pitch, -89.0f, 89.0f);

	// Update vectors
	const float radpitch = glm::radians(this->pitch);
	const float cospitch = std::cos(radpitch);
	const float radyaw   = glm::radians(this->yaw);

	this->forward = glm::normalize((glm::vec3){
		std::cos(radyaw) * cospitch,
		std::sin(radpitch),
		std::sin(radyaw) * cospitch
	});

	// Rebuild fixed positions
	this->right = glm::normalize(glm::cross(this->forward, { 0.0f, 1.0f, 0.0f }));
	this->up = glm::normalize(glm::cross(this->right, this->forward));
}


} // namespace floyd
