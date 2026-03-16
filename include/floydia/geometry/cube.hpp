#pragma once

#include <floydia/gfx/renderable.hpp>
#include <floydia/gfx/material.hpp>

namespace floyd {

struct Cube : public Renderable {
	// Used to define the faces to render
	enum Face : uint8 {
		FRONT  = 1 << 0, // -Z
		BACK   = 1 << 1, // +Z
		TOP    = 1 << 2, // +Y
		BOTTOM = 1 << 3, // -Y
		RIGHT  = 1 << 4, // +X
		LEFT   = 1 << 5, // -X
	};

	// Render all faces by default
	// Cube(const uint8 face_mask = 0b111111) noexcept;

	// TODO: not complete
	Cube(const std::shared_ptr<Material>& material) noexcept;
	static std::shared_ptr<Model> create_model(const std::shared_ptr<Material>& material) noexcept;
};

} // namespace floyd
