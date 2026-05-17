#pragma once

#include <floydia/rendering/renderable.hpp>
#include <floydia/material/texture.hpp>


namespace floyd {

class Sprite : public Renderable {
	public:
		Sprite() noexcept;

	protected:
		static std::shared_ptr<Model> create_model() noexcept;
};

} // namespace floyd
