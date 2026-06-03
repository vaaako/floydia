#include <floydia/rendering/renderable.hpp>

#include <floydia/helpers/logger.hpp>
#include <floydia/core/core.hpp>

namespace floyd {

Renderable::Renderable(const std::shared_ptr<Model>& model) noexcept
	: _model(std::move(model)) {}

glm::mat4 Renderable::final_matrix(const glm::mat4& view) const noexcept {
	(void)(view);
	return this->transform.model_matrix();
}

AABB Renderable::world_aabb() noexcept {
	// Also check for valid world_abb, so it is initialized the first time
	if(this->transform.isdirty() || !this->_world_aabb.valid)
		this->_world_aabb = this->_model->aabb.to_world(this->transform.model_matrix());
	return this->_world_aabb;
}

void Renderable::set_albedo_all(const std::shared_ptr<Texture>& tex) noexcept {
	for(auto& submesh : this->_model->meshes()) {
		submesh.material->albedo = tex;
	}
}

} // namespace floyd
