#pragma once

#include <floydia/rendering/renderable.hpp>
#include <floydia/material/texture.hpp>


namespace floyd {

class Sprite : public Renderable {
	public:
		Sprite() noexcept;

		// Sprite do not use AABB
		inline AABB world_aabb() noexcept override { return {}; }

	protected:
		static std::shared_ptr<Model> create_model() noexcept;
};

} // namespace floyd
