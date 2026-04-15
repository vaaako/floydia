#include <floydia/geometry/sprite.hpp>

#include <floydia/core/core.hpp>

namespace floyd {
	Sprite::Sprite() noexcept
		: Renderable(Sprite::create_model()) {}

	std::shared_ptr<Model> Sprite::create_model() noexcept {
		std::shared_ptr<Mesh> mesh = Core::instance->assets->quad_mesh;
		std::shared_ptr<Material> material = Core::instance->assets->default_material2d;
		std::shared_ptr<Model> model = std::make_shared<Model>();
		model->add_submesh(mesh, material); // use default material
		return model;
	}

} // namespace floyd
