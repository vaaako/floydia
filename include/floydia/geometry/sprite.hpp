#pragma once

#include <floydia/gfx/texture.hpp>
#include <floydia/gfx/renderable.hpp>


namespace floyd {

class Sprite final : public Renderable {
	public:
		Sprite() noexcept;

	private:
		static std::shared_ptr<Model> create_model() noexcept;
};

} // namespace floyd
