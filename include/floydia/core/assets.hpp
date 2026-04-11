#pragma once

#include <memory>
#include <floydia/gfx/mesh.hpp>
#include <floydia/gfx/material.hpp>
#include <floydia/gpu/shaders.hpp>
#include <floydia/core/buffermanager.hpp>


namespace floyd {

// TODO: instead of members: store in unordered_map

class Assets final {
	public:
		// Cube mesh
		std::shared_ptr<Mesh> cube_mesh;
		// Quad mesh
		std::shared_ptr<Mesh> quad_mesh;
		// Default Shader Program used by almost all models
		std::shared_ptr<ShaderProgram> program;
		// Default Shader Program using vertex shader 2D
		std::shared_ptr<ShaderProgram> program2d;
		// Shader Program with DEFAULT_VERTEX only
		std::shared_ptr<ShaderProgram> program_vertex;
		// Shader Program with DEFAULT_VERTEX_2D only
		std::shared_ptr<ShaderProgram> program_vertex2d;
		// Shader Program with DEFAULT_FRAGMENT only
		std::shared_ptr<ShaderProgram> program_fragment;
		// Material containing 'program_vertex' and 'program_fragment'
		std::shared_ptr<Material> default_material;
		// Material containing 'program_vertex2d' and 'program_fragment'
		std::shared_ptr<Material> default_material2d;

	public:
		Assets(BufferManager& bm) noexcept;
		~Assets() = default;

	private:
		std::shared_ptr<Mesh> make_cube_mesh() noexcept;
		std::shared_ptr<Mesh> make_quad_mesh() noexcept;
};

} // namespace floyd
