#include <floydia/geometry/cube.hpp>

#include <floydia/core/core.hpp>
#include <floydia/material/materialinstance.hpp>

namespace floyd {

Cube::Cube() noexcept : Renderable(Cube::create_model()) {}

std::shared_ptr<Model> Cube::create_model() noexcept {
	std::shared_ptr<Mesh> mesh = assets().load_cube_mesh();
	std::shared_ptr<MaterialInstance> matinst = std::make_shared<MaterialInstance>(
		assets().defaults.MAT_3D,
		assets().load<Texture>(hash::of("d_white"))
	);
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->add_submesh(mesh, matinst); // use default material
	return model;
}

} // namespace floyd
