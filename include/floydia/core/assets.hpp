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
		// Default shader used by almost all models
		static inline std::shared_ptr<ShaderProgram> default_program() noexcept {
			static std::shared_ptr<ShaderProgram> program = BufferManager::get().load_shaderprogram(
				Shaders::DEFAULT_VERTEX, Shaders::DEFAULT_FRAGMENT
			);
			return program;
		}

		// Default shader used by 2D models
		static inline std::shared_ptr<ShaderProgram> default_program2d() noexcept {
			static std::shared_ptr<ShaderProgram> program = BufferManager::get().load_shaderprogram(
				Shaders::DEFAULT_VERTEX_2D, Shaders::DEFAULT_FRAGMENT
			);
			return program;
		}

		// Default Material
		static inline std::shared_ptr<Material> default_material() noexcept {
			static std::shared_ptr<Material> material = std::make_shared<Material>(Assets::default_program());
			return material;
		}

		// Default Material 2D
		static inline std::shared_ptr<Material> default_material2d() noexcept {
			static std::shared_ptr<Material> material = std::make_shared<Material>(Assets::default_program2d());
			return material;
		}

		static std::shared_ptr<Mesh> cube_mesh() noexcept;
		static std::shared_ptr<Mesh> quad_mesh() noexcept;
	//	std::shared_ptr<Texture> load_texture(const std::string& path) noexcept;
	//private:
	//	std::unordered_map<std::string, std::weak_ptr<Mesh>> meshes;
	//	std::unordered_map<std::string, std::weak_ptr<Texture>> textures;
	//	std::unordered_map<std::string, std::weak_ptr<Shader>> shaders;
	//	std::unordered_map<std::string, std::weak_ptr<ShaderProgram>> shaderprograms;
	//	std::unordered_map<std::string, std::weak_ptr<ShaderProgram>> materials;
};

} // namespace floyd
