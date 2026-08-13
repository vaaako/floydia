#pragma once

#include "floydia/rendering/renderable.hpp"

namespace floyd {

class Billboard : public Renderable {
	public:
		enum Type : u8 {
			// Rotates all axis
			Full = 0,
			// Rotates only Y axis
			Cylindrical = 1
		};

		Billboard::Type type = Billboard::Type::Full;
	
	public:
		Billboard() noexcept;
		// Update Billboard AABB
		AABB world_aabb() noexcept override;

		// The type of this billboard.
		// Used to send data to GPU
		float billboard_type() const noexcept override { return static_cast<float>(this->type); }

	protected:
		static std::shared_ptr<Model> create_model() noexcept;
};

} // namespace floyd
