#pragma once

#include "floydia/rendering/renderable.hpp"

namespace floyd {

class Cube : public Renderable {
	public:
		// Used to define the faces to render
		enum Face : u8 {
			FRONT  = 1 << 0, // -Z
			BACK   = 1 << 1, // +Z
			TOP    = 1 << 2, // +Y
			BOTTOM = 1 << 3, // -Y
			RIGHT  = 1 << 4, // +X
			LEFT   = 1 << 5, // -X
		};

		// TODO: not complete

		// Render all faces by default
		// Cube(const u8 face_mask = 0b111111) noexcept;

		Cube() noexcept;

	// This is static so method is not repeated
	protected:
		static std::shared_ptr<Model> create_model() noexcept;
};

} // namespace floyd
