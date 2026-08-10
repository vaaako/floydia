#include "floydia/geometry/billboard.hpp"
#include "floydia/core/core.hpp"
#include <algorithm>

namespace floyd {

Billboard::Billboard() noexcept : Renderable(Billboard::create_model()) {}

std::shared_ptr<Model> Billboard::create_model() noexcept {
	std::shared_ptr<Mesh> mesh = assets().load_quad3d_mesh();
	Material mat = Material(
		assets().load_program(Shaders::DEFAULT_BILLBOARD_VERTEX, nullptr),
		assets().progs.PROG_FRAG_3D
	);
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->add_submesh(mesh, mat); // use default material
	return model;
}

AABB Billboard::world_aabb() noexcept {
	if(!this->transform.isdirty() && this->_world_aabb.valid) return this->_world_aabb;

	const vec3<float> pos   = this->transform.position();
	const vec3<float> scale = this->transform.scale();
	const vec3<float> ext   = this->_model->aabb.extents(); // half size of local AABB
	
	if(this->type == Billboard::Full) {
		// Extracts half size and multiply by the object scale
		// making this a uniform radius regardless of the AABB rotation.
		// Can't be used a directional AABB like regular objects
		const float radius = std::max({ ext.x * scale.x, ext.y * scale.y, ext.z * scale.z });
		// Symmetrical cube centered on world position
		this->_world_aabb.min   = { pos.x - radius, pos.y - radius, pos.z - radius };
		this->_world_aabb.max   = { pos.x + radius, pos.y + radius, pos.z + radius };
		// Otherwhise AABB would be a plane
	} else {
		const float radius = std::max({ ext.x * scale.x, ext.z * scale.z });
		const float half_height = ext.y * scale.y;
		this->_world_aabb.min = { pos.x - radius, pos.y - half_height, pos.z - radius };
		this->_world_aabb.max = { pos.x + radius, pos.y + half_height, pos.z + radius };
	}

	this->_world_aabb.valid = true;
	return this->_world_aabb;
}

} // namespace floyd
