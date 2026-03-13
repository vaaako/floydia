#pragma once

#include <floydia/gfx/texture.hpp>
#include <floydia/gfx/renderable.hpp>


namespace floyd {

class Sprite : public Renderable {
	public:
		//Sprite(const std::shared_ptr<Texture>& texture);
		// TODO: Renderable(Assets::get_quad_mesh(), Assets::get_sprite_material(texture))

    // static std::shared_ptr<Model> create_model(const std::shared_ptr<Texture>& texture)
    // {
    //     auto model = std::make_shared<Model>();

    //     auto mesh = Assets::quad_mesh();      // quad compartilhado
    //     auto material = std::make_shared<Material>();

    //     material->set_texture(texture);

    //     model->add_submesh(mesh, material);

    //     return model;
    // }
}

} // namespace floyd
