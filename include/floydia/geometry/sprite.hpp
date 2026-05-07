#pragma once

#include <floydia/rendering/renderable.hpp>
#include <floydia/material/texture.hpp>


namespace floyd {

class Sprite final : public Renderable {
	public:
		Sprite() noexcept;

	private:
		static std::shared_ptr<Model> create_model() noexcept;
};

} // namespace floyd
