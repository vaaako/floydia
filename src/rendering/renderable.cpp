#include <floydia/rendering/renderable.hpp>

#include <floydia/helpers/logger.hpp>
#include <floydia/core/core.hpp>

namespace floyd {

Renderable::Renderable(const std::shared_ptr<Model>& model) noexcept
	: _model(std::move(model)) {}

AABB Renderable::world_aabb() noexcept {
	// Also check for valid world_abb, so it is initialized the first time
	if(this->transform.isdirty() || !this->_world_aabb.valid)
		this->_world_aabb = this->_model->aabb.to_world(this->transform.model_matrix());
	return this->_world_aabb;
}

void Renderable::set_albedo(const std::shared_ptr<Texture>& albedo, const size_t index) noexcept {
	this->material(index).albedo = (albedo != nullptr) ? albedo : assets().load<Texture>(hash::of("d_white"));
	if(this->is_persistent) this->needs_rebatch = true;
}

void Renderable::set_albedo_all(const std::shared_ptr<Texture>& tex) noexcept {
	for(Model::SubMesh& submesh : this->_model->meshes()) {
		submesh.material.albedo = tex;
	}
}

} // namespace floyd
