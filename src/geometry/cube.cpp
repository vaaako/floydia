#include <floydia/geometry/cube.hpp>

#include <floydia/core/assets.hpp>

namespace floyd {

Cube::Cube() noexcept
	: Renderable(Cube::create_model()) {}

std::shared_ptr<Model> Cube::create_model() noexcept {
	std::shared_ptr<Mesh> mesh = Assets::cube_mesh();
	std::shared_ptr<Model> model = std::make_shared<Model>();
	std::shared_ptr<Material> material = std::make_shared<Material>(Assets::default_program());
	model->add_submesh(mesh, material); // use default material
	return model;
}

} // namespace floyd
