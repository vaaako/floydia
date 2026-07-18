#include "floydia/rendering/model.hpp"

namespace floyd {

void Model::add_submesh(const std::shared_ptr<Mesh>& mesh, const Material& mat) {
	if(mesh == nullptr) throw std::invalid_argument("'mesh' cannot be nullptr when building a SubMesh");
	this->_submeshes.push_back({ mesh, mat });
	this->aabb.merge(mesh->aabb);
}

} // namespace floyd
