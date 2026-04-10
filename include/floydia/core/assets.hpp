#pragma once

#include <memory>
#include <floydia/gfx/mesh.hpp>
#include <floydia/gfx/material.hpp>
#include <floydia/gpu/shaders.hpp>
#include <floydia/core/buffermanager.hpp>


namespace floyd {

// TODO: instead of static method: store in unordered_map

class Assets final {
	public:
		// Default Shader Program used by almost all models
		static inline std::shared_ptr<ShaderProgram> default_program() noexcept {
			static std::shared_ptr<ShaderProgram> program = BufferManager::get().load_shaderprogram(
				Shaders::DEFAULT_VERTEX, Shaders::DEFAULT_FRAGMENT
			);
			return program;
		}

		// Default Shader Program using vertex shader 2D
		static inline std::shared_ptr<ShaderProgram> default_program2d() noexcept {
			static std::shared_ptr<ShaderProgram> program = BufferManager::get().load_shaderprogram(
				Shaders::DEFAULT_VERTEX_2D, Shaders::DEFAULT_FRAGMENT
			);
			return program;
		}

		// Default Shader Program with DEFAULT_VERTEX
		static inline std::shared_ptr<ShaderProgram> default_program_vertex() noexcept {
			static std::shared_ptr<ShaderProgram> program = BufferManager::get().load_shaderprogram(
				Shaders::DEFAULT_VERTEX, nullptr
			);
			return program;
		}

		// Default Shader Program with DEFAULT_VERTEX_2D
		static inline std::shared_ptr<ShaderProgram> default_program_vertex2d() noexcept {
			static std::shared_ptr<ShaderProgram> program = BufferManager::get().load_shaderprogram(
				Shaders::DEFAULT_VERTEX_2D, nullptr
			);
			return program;
		}

		// Default Shader Program with DEFAULT_FRAGMENT
		static inline std::shared_ptr<ShaderProgram> default_program_fragment() noexcept {
			static std::shared_ptr<ShaderProgram> program = BufferManager::get().load_shaderprogram(
				nullptr, Shaders::DEFAULT_FRAGMENT
			);
			return program;
		}

		// Default Material
		static inline std::shared_ptr<Material> default_material() noexcept {
			static std::shared_ptr<Material> material = std::make_shared<Material>(
				Assets::default_program_vertex(),
				Assets::default_program_fragment()
			);
			return material;
		}

		// Default Material 2D
		static inline std::shared_ptr<Material> default_material2d() noexcept {
			static std::shared_ptr<Material> material = std::make_shared<Material>(
				Assets::default_program_vertex2d(),
				Assets::default_program_fragment()
			);
			return material;
		}

		static std::shared_ptr<Mesh> cube_mesh() noexcept;
		static std::shared_ptr<Mesh> quad_mesh() noexcept;
	//	std::shared_ptr<Texture> load_texture(const std::string& path) noexcept;
	//private:
	//	std::unordered_map<std::string, std::weak_ptr<Texture>> textures;
};

} // namespace floyd
