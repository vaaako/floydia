#pragma once

#include <memory>
#include <vector>
#include <floydia/gfx/material.hpp>
#include <floydia/gfx/mesh.hpp>

#include <stdexcept>

namespace floyd {

// Stores all Meshes of a Model
class Model final {
	public:
		struct SubMesh {
			std::shared_ptr<Mesh> mesh;
			std::shared_ptr<Material> material;
		};

		inline void add_submesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material) {
			if(mesh == nullptr) {
				throw std::invalid_argument("mesh can not be nullptr");
			}
			this->_submeshes.push_back({
				mesh,
				(material != nullptr) ? material : std::make_shared<Material>()
			});
		}

		inline const std::vector<SubMesh>& meshes() const noexcept {
			return this->_submeshes;
		}

	private:
		std::vector<SubMesh> _submeshes;
};

} // namespace floyd
