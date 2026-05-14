#include <floydia/geometry/sprite.hpp>

#include <floydia/core/core.hpp>
#include <floydia/material/materialinstance.hpp>

namespace floyd {

Sprite::Sprite() noexcept : Renderable(Sprite::create_model()) {}

std::shared_ptr<Model> Sprite::create_model() noexcept {
	Assets* assets = Core::get().assets.get();
	std::shared_ptr<Mesh> mesh = Core::get().assets->load<Mesh>("quad");
	std::shared_ptr<MaterialInstance> matinst = std::make_shared<MaterialInstance>(
		assets->load<Material>(assets->hashes.MAT_2D),
		assets->load_texture("d_white")
	);
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->add_submesh(mesh, matinst); // use default material
	return model;
}

} // namespace floyd
