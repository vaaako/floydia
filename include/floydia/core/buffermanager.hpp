#pragma once

#include <unordered_map>
#include <floydia/gpu/shader.hpp>
// #include <floydia/gpu/shaders.hpp>
#include <floydia/gpu/shaderprogram.hpp>

namespace floyd {

class BufferManager {
	public:
		static inline BufferManager& get() {
			static BufferManager bm;
			return bm;
		}

		// NOTE: Not the proper place i think
		// Default shader used by almost all models
		// static inline std::shared_ptr<ShaderProgram> default_program() noexcept {
		// 	static std::shared_ptr<ShaderProgram> program = BufferManager::get().load_shaderprogram(
		// 		Shaders::DEFAULT_VERTEX, Shaders::DEFAULT_FRAGMENT
		// 	);
		// 	return program;
		// }

		// Checks for an existing Vertex Array.
		// Returns an existing Vertex Array or uploads a newly created if it didn't

		// Checks if an existing Shader Program (same for the Shaders inside it).
		// Returns an existing Shader Program or uploads a newly created if it didn't
		std::shared_ptr<ShaderProgram> load_shaderprogram(const char* vertex, const char* fragment);
		// Checks if an existing Shader.
		// Returns an existing Shader or uploads a newly created if it didn't
		std::shared_ptr<Shader> load_shader(const char* source, const Shader::Type type);

		// Get an existing Shader Program using its hash.
		// Returns 'nullptr' if not found
		std::shared_ptr<ShaderProgram> get_shaderprogram(const size_t hash) noexcept;
		// Get an existing Shader using its hash.
		// Returns 'nullptr' if not found
		std::shared_ptr<Shader> get_shader(const size_t hash) noexcept;


		// void cleanup() noexcept;
	private:
		// std::unordered_map<size_t, std::weak_ptr<VertexArray>> vertexarray_cache;
		std::unordered_map<size_t, std::weak_ptr<ShaderProgram>> shaderprogram_cache;
		std::unordered_map<size_t, std::weak_ptr<Shader>> shader_cache;
};

} // namespace floyd
