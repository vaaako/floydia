#pragma once

#include "floydia/geometry/font.hpp"
#include "floydia/gpu/shaderprogram.hpp"
#include "floydia/rendering/mesh.hpp"
#include "floydia/material/texture.hpp"
#include <unordered_map>
#include <memory>

namespace floyd {

// Keep alive assets pointers
class Assets final {
	public:
		// Default Shader Programs
		struct Default {
			// Shader Program of Shaders::DEFAULT_VERTEX
			std::shared_ptr<ShaderProgram> PROG_VERT_3D;
			// Shader Program of Shaders::DEFAULT_VERTEX_2D
			std::shared_ptr<ShaderProgram> PROG_VERT_2D;
			// Shader Program of Shaders::DEFAULT_FRAGMENT
			std::shared_ptr<ShaderProgram> PROG_FRAG_3D;
			// Shader Program of Shaders::DEFAULT_FRAGMENT_2D
			std::shared_ptr<ShaderProgram> PROG_FRAG_2D;
			// Shader Program of Shaders::DEFAULT_VERTEX_TEXT
			std::shared_ptr<ShaderProgram> PROG_VERT_TEXT;
		} progs;

	public:
		Assets() noexcept;
		~Assets() noexcept = default;

		// Builds a new Texture or returns an existing one
		std::shared_ptr<Texture> load_texture(const char* path) noexcept;
		// Builds a new Texture or returns an existing one.
		// 'name' represents the name the texture will be stored with
		std::shared_ptr<Texture> load_texture(u8* data, const u32 width, const u32 height, const u8 channels = 4, const char* name = "[DATA_TEXTURE]") noexcept;
		// Loads a cell from a spritesheet
		std::shared_ptr<Texture> load_spritesheet_cell(const std::shared_ptr<Texture>& sheet,
			const u32 frame_width, const u32 frame_height, const u32 index);

		// Builds a new TTF file or returns an existing one.
		std::shared_ptr<Font> load_font(const char* path, const u32 size);
		// Builds a new Shader Program or returns an existing one.
		// If Vertex or Fragment shader is null, the Shader Program will be separable
		std::shared_ptr<ShaderProgram> load_program(const char* vertex, const char* fragment);
		// Builds a Model of a Wavefront OBJ file or returns if already loaded
		std::shared_ptr<Model> load_model(const char* path);

		// Load a quad mesh
		std::shared_ptr<Mesh> load_quad_mesh() noexcept;
		// Load a quad mesh with normals
		std::shared_ptr<Mesh> load_quad3d_mesh() noexcept;
		// Load a Cube mesh
		std::shared_ptr<Mesh> load_cube_mesh() noexcept;

		// Load an existing Asset
		template <typename T>
		std::shared_ptr<T> load(const size_t hash);

	private:
		struct TextureEntry {
			std::shared_ptr<Texture> texture;
			std::string name; // Useful for picking
		};

	private:
		std::unordered_map<size_t, TextureEntry> textures;
		std::unordered_map<size_t, std::shared_ptr<Font>> texts;
		std::unordered_map<size_t, std::shared_ptr<ShaderProgram>> programs;
		std::unordered_map<size_t, std::shared_ptr<Model>> models;
	
	private:
		template <typename T, typename Factory>
		std::shared_ptr<T> find_or_create(std::unordered_map<size_t, std::shared_ptr<T>>& cache,
			const size_t key, Factory&& factory) noexcept;

		// Specialization
		template <typename T> auto& get_cache();
};

template <typename T, typename Factory>
std::shared_ptr<T> Assets::find_or_create(std::unordered_map<size_t, std::shared_ptr<T>>& cache,
	const size_t key, Factory&& factory) noexcept {

	auto it = cache.find(key);
	if(it != cache.end()) return it->second;

	std::shared_ptr<T> obj = factory();
	cache[key] = obj;
	return obj;
}



template<> inline auto& Assets::get_cache<Texture>() { return this->textures; }
template<> inline auto& Assets::get_cache<Font>() { return this->texts; }
template<> inline auto& Assets::get_cache<ShaderProgram>() { return this->programs; }
template<> inline auto& Assets::get_cache<Model>() { return this->models; }

template <typename T>
std::shared_ptr<T> Assets::load(const size_t hash) {
	auto& cache = this->get_cache<T>();
	auto it = cache.find(hash);
	if(it != cache.end()) return it->second;
	return nullptr;
}

template <>
inline std::shared_ptr<Texture> Assets::load(const size_t hash) {
	auto it = this->textures.find(hash);
	if(it != this->textures.end()) return it->second.texture;
	return nullptr;
}

} // namespace floyd
