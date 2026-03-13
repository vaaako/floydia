#include <floydia/gfx/renderable.hpp>

namespace floyd {

Renderable::Renderable(const std::shared_ptr<Model>& model) noexcept : _model(std::move(model)) {}

void Renderable::submit(Renderer& renderer) const noexcept {
	glm::mat4 model_matrix = this->transform.model_matrix();
	for(const Model::SubMesh& sub : this->_model->meshes()) {
		renderer.submit({ sub.mesh.get(), sub.material.get(), model_matrix });
	}
}

} // namespace floyd
