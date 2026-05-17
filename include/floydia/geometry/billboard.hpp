#pragma once

#include <floydia/rendering/renderable.hpp>

namespace floyd {

class Billboard : public Renderable {
	public:
		enum Type : uint8 {
			// Rotates all axis
			Full = 1,
			// Rotates only Y axis
			Cylindrical = 2
		};

		Type type = Type::Full;
	
	public:
		Billboard() noexcept;
		// Calculates model matrix
		glm::mat4 final_matrix(const glm::mat4& view) const noexcept override;

	protected:
		static std::shared_ptr<Model> create_model() noexcept;
};

} // namespace floyd
