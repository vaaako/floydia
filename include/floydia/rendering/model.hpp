#pragma once

#include <memory>
#include <vector>
#include <floydia/rendering/mesh.hpp>
#include <floydia/material/materialinstance.hpp>

namespace floyd {

// Stores all Meshes of a Model
class Model final {
	public:
		AABB aabb;
		
		struct SubMesh {
			std::shared_ptr<Mesh> mesh;
			std::shared_ptr<MaterialInstance> material;
		};

	public:
		void add_submesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<MaterialInstance>& matinst);
		// Returns all meshes on Model
		inline const std::vector<SubMesh>& meshes() const noexcept { return this->_submeshes; }
		// Returns all meshes on Model
		inline std::vector<SubMesh>& meshes() noexcept { return this->_submeshes; }
		// Method: Combine all submeshes AABB
	private:
		std::vector<SubMesh> _submeshes;
};

} // namespace floyd
