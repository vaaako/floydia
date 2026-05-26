#include <floydia/rendering/renderable.hpp>

#include <floydia/helpers/logger.hpp>
#include <floydia/core/core.hpp>

namespace floyd {

Renderable::Renderable(const std::shared_ptr<Model>& model) noexcept
	: _model(std::move(model)) {}
Renderable::Renderable(const Renderable& other) noexcept
	: Object(other) , visible(other.visible) , is_persitent(other.is_persitent) , _world_aabb(other._world_aabb) , _model(other._model) {}

void Renderable::update_scripts(const float dt) noexcept {
	for(auto& s : this->scripts) s->update(dt);
}

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

} // namespace floyd
