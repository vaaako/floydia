#pragma once

#include <memory>
#include <vector>
#include "floydia/material/material.hpp"
#include "floydia/rendering/mesh.hpp"

namespace floyd {

// Stores all Meshes of a Model
class Model final {
	public:
		AABB aabb;
		
		struct SubMesh {
			std::shared_ptr<Mesh> mesh;
			Material material;
			// NOTE: Not pointer anymore
			// - less heap allocation and access
			// - solves the problem of shared Material
		};

	public:
		void add_submesh(const std::shared_ptr<Mesh>& mesh, const Material& matinst);
		// Returns all meshes on Model
		inline const std::vector<SubMesh>& meshes() const noexcept { return this->_submeshes; }
		// Returns all meshes on Model
		inline std::vector<SubMesh>& meshes() noexcept { return this->_submeshes; }
		// Method: Combine all submeshes AABB
	private:
		std::vector<SubMesh> _submeshes;
};

} // namespace floyd
