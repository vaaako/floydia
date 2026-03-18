#include <floydia/geometry/sprite.hpp>

namespace floyd {
	Sprite::Sprite() noexcept
		: Renderable(Sprite::create_model()) {}

	std::shared_ptr<Model> Sprite::create_model() noexcept {
		std::shared_ptr<Mesh> mesh = Assets::quad_mesh();
		std::shared_ptr<Model> model = std::make_shared<Model>();
		std::shared_ptr<Material> material = std::make_shared<Material>(Assets::default_program2d());
		model->add_submesh(mesh, material);
		return model;
	}

} // namespace floyd
