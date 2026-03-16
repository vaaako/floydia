#include <floydia/geometry/cube.hpp>

#include <floydia/core/assets.hpp>

namespace floyd {

Cube::Cube(const std::shared_ptr<Material>& material) noexcept
	: Renderable(Cube::create_model(material)) {}

std::shared_ptr<Model> Cube::create_model(const std::shared_ptr<Material>& material) noexcept {
	std::shared_ptr<Mesh> mesh = Assets::cube_mesh();
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->add_submesh(mesh, material);
	return model;
}

} // namespace floyd
