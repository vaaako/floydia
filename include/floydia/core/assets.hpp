#pragma once

#include "floydia/geometry/text.hpp"
#include "floydia/helpers/hash.hpp"
#include "floydia/rendering/model.hpp"
#include "floydia/gpu/shaderprogram.hpp"
#include "floydia/rendering/mesh.hpp"
#include "floydia/material/material.hpp"
#include "floydia/material/texture.hpp"
#include <unordered_map>
#include <memory>

namespace floyd {

// Keep alive BufferManager pointers.
// User-friendly wrapper for BufferManager.
// May contain repeated Assets if user pushes same asset with different name
class Assets final {
	public:
		// Default resource hashes. Computed once, used for fast lookup
		struct Hashes {
			// Shader Program of Shaders::DEFAULT_VERTEX
			size_t PROG_VERT_3D = 0;
			// Shader Program of Shaders::DEFAULT_VERTEX_2D
			size_t PROG_VERT_2D = 0;
			// Shader Program of Shaders::DEFAULT_FRAGMENT
			size_t PROG_FRAG_3D = 0;
			// Shader Program of Shaders::DEFAULT_FRAGMENT_2D
			size_t PROG_FRAG_2D = 0;
			// Shader Program of Shaders::DEFAULT_VERTEX_TEXT
			size_t PROG_VERT_TEXT = 0;
			// Shader Program of Shaders::DEFAULT_FRAGMENT_TEXT
			size_t PROG_FRAG_TEXT = 0;
			// Material of Shaders::DEFAULT_VERTEX + Shaders::DEFAULT_FRAGMENT
			size_t MAT_3D = 0;
			// Material of Shaders::DEFAULT_VERTEX_2D + Shaders::DEFAULT_FRAGMENT_2D
			size_t MAT_2D = 0;
			// Material of Shaders::DEFAULT_VERTEX_TEXT + Shaders::DEFAULT_FRAGMENT_TEXT
			size_t MAT_TEXT = 0;
		} hashes;

	public:
		Assets() noexcept;
		~Assets() noexcept = default;

		// Builds a new Texture or returns an existing one
		std::shared_ptr<Texture> load_texture(const char* path);
		// Builds a new Texture or returns an existing one
		std::shared_ptr<Texture> load_texture(uint8* data, const uint32 width, const uint32 height, const uint8 channels = 4);
		// Builds a new TTF file or returns an existing one.
		std::shared_ptr<Text> load_font(const char* path, const uint32 size);
		// Builds a new Shader Program or returns an existing one.
		// If Vertex or Fragment shader is null, the Shader Program will be separable
		std::shared_ptr<ShaderProgram> load_program(const char* vertex, const char* fragment);
		// Builds a new Material or returns an existing one
		std::shared_ptr<Material> load_material(
			const std::shared_ptr<ShaderProgram>& vertex,
			const std::shared_ptr<ShaderProgram>& fragment
		) noexcept;
		// Builds a Model of a Wavefront OBJ file or returns if already loaded
		std::shared_ptr<Model> load_model(const char* path);

		// Load a quad mesh
		std::shared_ptr<Mesh> load_quad_mesh() noexcept;
		// Load a quad mesh with normals
		std::shared_ptr<Mesh> load_quad3d_mesh() noexcept;
		// Load a Cube mesh
		std::shared_ptr<Mesh> load_cube_mesh() noexcept;

		// Load an existing Shader Program, Material, Texture or Model
		template <typename T>
		std::shared_ptr<T> load(const size_t hash);
		// Load an existing Shader Program, Material, Texture, Font or Model
		template <typename T>
		std::shared_ptr<T> load(const std::string_view& key);

	private:
		std::unordered_map<size_t, std::shared_ptr<Texture>> textures;
		std::unordered_map<size_t, std::shared_ptr<Text>> texts;
		std::unordered_map<size_t, std::shared_ptr<ShaderProgram>> programs;
		std::unordered_map<size_t, std::shared_ptr<Material>> materials;
		std::unordered_map<size_t, std::shared_ptr<Model>> models;

		// Specialization
		template <typename T> auto& get_cache();

		// Helper to make a hash of a Material
		size_t material_hash(const std::shared_ptr<ShaderProgram>& vertex, const std::shared_ptr<ShaderProgram>& fragment) const noexcept;
};

template<> inline auto& Assets::get_cache<Texture>() { return this->textures; }
template<> inline auto& Assets::get_cache<Text>() { return this->texts; }
template<> inline auto& Assets::get_cache<ShaderProgram>() { return this->programs; }
template<> inline auto& Assets::get_cache<Material>() { return this->materials; }
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
