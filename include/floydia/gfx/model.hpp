#pragma once

#include <memory>
#include <vector>
#include <floydia/gfx/material.hpp>
#include <floydia/gfx/mesh.hpp>


namespace floyd {

class Model final {
	public:
		struct SubMesh {
			std::shared_ptr<Mesh> mesh;
			std::shared_ptr<Material> material;
		};

		inline void add_submesh(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material) noexcept {
			this->_submeshes.push_back({ mesh, material });
		}

		inline const std::vector<SubMesh>& meshes() const noexcept {
			return this->_submeshes;
		}

	private:
		std::vector<SubMesh> _submeshes;
};

} // namespace floyd
