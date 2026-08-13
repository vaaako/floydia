#include "floydia/geometry/sprite.hpp"
#include "floydia/core/core.hpp"

namespace floyd {

Sprite::Sprite() noexcept : Renderable(Sprite::create_model()) {}

std::shared_ptr<Model> Sprite::create_model() noexcept {
	std::shared_ptr<Mesh> mesh = assets().load_quad_mesh();
	Material mat = Material(assets().progs.PROG_VERT_2D, assets().progs.PROG_FRAG_2D);
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->add_submesh(mesh, mat); // use default material
	return model;
}

} // namespace floyd
