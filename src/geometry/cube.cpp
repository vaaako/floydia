#include <floydia/geometry/cube.hpp>

#include <floydia/core/core.hpp>
#include <floydia/material/materialinstance.hpp>

namespace floyd {

Cube::Cube() noexcept
	: Renderable(Cube::create_model()) {}

std::shared_ptr<Model> Cube::create_model() noexcept {
	Assets* assets = Core::get().assets.get();
	std::shared_ptr<Mesh> mesh = assets->load<Mesh>("cube");
	std::shared_ptr<Material> material = assets->load_material(assets->load_program(Shaders::DEFAULT_VERTEX, nullptr), assets->load_program(nullptr, Shaders::DEFAULT_FRAGMENT));
	std::shared_ptr<MaterialInstance> matinst = std::make_shared<MaterialInstance>(
		material,
		assets->load_texture("d_white")
	);
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->add_submesh(mesh, matinst); // use default material
	return model;
}

} // namespace floyd
