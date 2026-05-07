#include <floydia/rendering/model.hpp>

namespace floyd {

void Model::add_submesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<MaterialInstance>& matinst) {
	if(mesh == nullptr || matinst == nullptr) throw std::invalid_argument("mesh or matinst cannot be nullptr");
	this->_submeshes.push_back({ mesh, matinst });
	this->aabb.merge(mesh->aabb);
}

} // namespace floyd
