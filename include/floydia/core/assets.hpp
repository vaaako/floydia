#pragma once

#include <floydia/rendering/mesh.hpp>
#include <floydia/rendering/material.hpp>
#include <floydia/gpu/shaders.hpp>
#include <unordered_map>
#include <memory>

namespace floyd {

// Keep alive BufferManager pointers.
// User-friendly wrapper for BufferManager.
// May contain repeated Assets if user pushes same asset with different name
class Assets final {
	public:
		Assets() noexcept;
		~Assets() = default;

		// Try to return a stored Mesh. Returns nullptr if not found
		inline std::shared_ptr<Mesh> get_mesh(const std::string& key) const noexcept { return this->get_asset(key, this->meshes); }
		// Try to return a stored Shader Program. Returns nullptr if not found
		inline std::shared_ptr<ShaderProgram> get_program(const std::string& key) const noexcept { return this->get_asset(key, this->programs); }
		// Try to return a stored Material. Returns nullptr if not found
		inline std::shared_ptr<Material> get_material(const std::string& key) const noexcept { return this->get_asset(key, this->materials); }

		// Builds a new Shader Program or pushes an existing and return it.
		std::shared_ptr<ShaderProgram> emplace_program(const std::string& key, const char* vertex, const char* fragment);

	private:
		// Putting on map is optional
		std::shared_ptr<Mesh> make_cube_mesh() noexcept;
		std::shared_ptr<Mesh> make_quad_mesh() noexcept;

		std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
		std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> programs;
		std::unordered_map<std::string, std::shared_ptr<Material>> materials;

		// Helper to get obj from maps
		template <typename T>
		std::shared_ptr<T> get_asset(const std::string& key, const std::unordered_map<std::string, std::shared_ptr<T>>& map) const noexcept;
};

template <typename T>
std::shared_ptr<T> Assets::get_asset(const std::string& key, const std::unordered_map<std::string, std::shared_ptr<T>>& map) const noexcept {
	auto it = map.find(key);
	if(it == map.end()) return nullptr;
	return it->second;
}


} // namespace floyd
