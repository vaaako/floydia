#pragma once

#include <memory>
#include <unordered_map>
#include <floydia/gfx/mesh.hpp>
#include <floydia/gpu/shaders.hpp>
#include <floydia/core/buffermanager.hpp>

namespace floyd {

class Assets final {
	public:
		// Default shader used by almost all models
		static inline std::shared_ptr<ShaderProgram> default_program() noexcept {
			static std::shared_ptr<ShaderProgram> program = BufferManager::get().load_shaderprogram(
				Shaders::DEFAULT_VERTEX, Shaders::DEFAULT_FRAGMENT
			);
			return program;
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
