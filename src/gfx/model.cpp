#include <floydia/rendering/model.hpp>

namespace floyd {

void Model::add_submesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material) {
	if(mesh == nullptr || material == nullptr) {
		throw std::invalid_argument("mesh or material cannot be nullptr");
	}
	this->_submeshes.push_back({ mesh, material });
}

} // namespace floyd
