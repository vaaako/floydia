#pragma once

#include <floydia/helpers/hash.hpp>
#include <floydia/rendering/model.hpp>
#include <floydia/gpu/shaderprogram.hpp>
#include <floydia/rendering/mesh.hpp>
#include <floydia/material/material.hpp>
#include <floydia/material/texture.hpp>
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

		// Builds a new Texture or returns an existing one
		std::shared_ptr<Texture> load_texture(const char* path);
		// Builds a new Shader Program or returns an existing one.
		// If Vertex or Fragment shader is null, the Shader Program will be separable
		std::shared_ptr<ShaderProgram> load_program(const char* vertex, const char* fragment);
		// Builds a new Material or returns an existing one
		std::shared_ptr<Material> load_material(
			const std::shared_ptr<ShaderProgram>& vertex,
			const std::shared_ptr<ShaderProgram>& fragment
		) noexcept;

		std::shared_ptr<Model> load_model(const char* path);

		// Load an existing Mesh, Material, Texture or ShaderProgram
		template <typename T>
		std::shared_ptr<T> load(const size_t hash);
		template <typename T>
		std::shared_ptr<T> load(const std::string_view& key);

	private:
		// NOTE: Putting mesh on map just for user to access it
		std::shared_ptr<Mesh> make_quad_mesh() noexcept;
		std::shared_ptr<Mesh> make_quad3d_mesh() noexcept;
		std::shared_ptr<Mesh> make_cube_mesh() noexcept;

		std::unordered_map<size_t, std::shared_ptr<Mesh>> meshes;
		std::unordered_map<size_t, std::shared_ptr<ShaderProgram>> programs;
		std::unordered_map<size_t, std::shared_ptr<Material>> materials;
		std::unordered_map<size_t, std::shared_ptr<Texture>> textures;
		std::unordered_map<size_t, std::shared_ptr<Model>> models;

		// Specialization
		template <typename T> auto& get_cache();

		// Helper to make a hash of a Material
		size_t material_hash(const std::shared_ptr<ShaderProgram>& vertex, const std::shared_ptr<ShaderProgram>& fragment) const noexcept;
};

template<> inline auto& Assets::get_cache<Mesh>() { return this->meshes; }
template<> inline auto& Assets::get_cache<Texture>() { return this->textures; }
template<> inline auto& Assets::get_cache<Material>() { return this->materials; }
template<> inline auto& Assets::get_cache<ShaderProgram>() { return this->programs; }
template<> inline auto& Assets::get_cache<Model>() { return this->models; }


template <typename T>
std::shared_ptr<T> Assets::load(const std::string_view& key) { return this->load<T>(hash::of(key)); }

template <typename T>
std::shared_ptr<T> Assets::load(const size_t hash) {
	auto& cache = this->get_cache<T>();
	auto it = cache.find(hash);
	if(it != cache.end()) return it->second;
	return nullptr;
}


} // namespace floyd
