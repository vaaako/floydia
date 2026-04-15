#include <floydia/geometry/cube.hpp>

#include <floydia/core/core.hpp>

namespace floyd {

Cube::Cube() noexcept
	: Renderable(Cube::create_model()) {}

std::shared_ptr<Model> Cube::create_model() noexcept {
	std::shared_ptr<Mesh> mesh = Core::instance->assets->cube_mesh;
	std::shared_ptr<Material> material = Core::instance->assets->default_material;
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->add_submesh(mesh, material); // use default material
	return model;
}

} // namespace floyd
