#pragma once

#include <memory>
#include <vector>
#include <floydia/gfx/material.hpp>
#include <floydia/gfx/mesh.hpp>

namespace floyd {

// Stores all Meshes of a Model
class Model final {
	public:
		struct SubMesh {
			std::shared_ptr<Mesh> mesh;
			std::shared_ptr<Material> material;
		};
		void add_submesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material);
		// Returns all meshes on Model
		inline const std::vector<SubMesh>& meshes() const noexcept {
			return this->_submeshes;
		}

	private:
		std::vector<SubMesh> _submeshes;
};

} // namespace floyd
