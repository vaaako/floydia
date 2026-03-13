#pragma once

#include <floydia/gfx/mesh.hpp>
#include <unordered_map>
#include <memory>

namespace floyd {

class Assets final {
	public:
		static std::shared_ptr<Mesh> cube_mesh();
	//	std::shared_ptr<Texture> load_texture(const std::string& path) noexcept;
	//private:
	//	std::unordered_map<std::string, std::weak_ptr<Mesh>> meshes;
	//	std::unordered_map<std::string, std::weak_ptr<Texture>> textures;
	//	std::unordered_map<std::string, std::weak_ptr<Shader>> shaders;
	//	std::unordered_map<std::string, std::weak_ptr<ShaderProgram>> shaderprograms;
	//	std::unordered_map<std::string, std::weak_ptr<ShaderProgram>> materials;
};

} // namespace floyd
