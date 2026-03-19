#include <floydia/geometry/sprite.hpp>

#include <floydia/core/assets.hpp>

namespace floyd {
	Sprite::Sprite() noexcept
		: Renderable(Sprite::create_model()) {}

	std::shared_ptr<Model> Sprite::create_model() noexcept {
		std::shared_ptr<Mesh> mesh = Assets::quad_mesh();
		std::shared_ptr<Material> material = Assets::default_material2d();
		std::shared_ptr<Model> model = std::make_shared<Model>();
		model->add_submesh(mesh, material);
		return model;
	}

} // namespace floyd
