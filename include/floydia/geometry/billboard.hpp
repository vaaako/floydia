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

	protected:
		static std::shared_ptr<Model> create_model() noexcept;
};

} // namespace floyd
