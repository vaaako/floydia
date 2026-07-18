#include "floydia/geometry/cube.hpp"
#include "floydia/core/core.hpp"

namespace floyd {

Cube::Cube() noexcept : Renderable(Cube::create_model()) {}

std::shared_ptr<Model> Cube::create_model() noexcept {
	std::shared_ptr<Mesh> mesh = assets().load_cube_mesh();
	Material mat = Material(assets().defaults.PROG_VERT_3D, assets().defaults.PROG_FRAG_3D);
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->add_submesh(mesh, mat); // use default material
	return model;
}

} // namespace floyd
