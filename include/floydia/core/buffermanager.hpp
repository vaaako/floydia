#pragma once

#include <memory>
#include <unordered_map>
#include <floydia/gpu/shader.hpp>
#include <floydia/gpu/shaderprogram.hpp>
#include <floydia/gpu/programpipeline.hpp>

namespace floyd {

class BufferManager {
	public:
		BufferManager() = default;

		// TODO: add array of shaders on shader program too

		// Returns an existing Shader Program or uploads a newly created and returns if it didn't.
		// Returns 'nullptr' for error
		std::shared_ptr<ShaderProgram> load_shaderprogram(const char* vertex, const char* fragment);
		// Get an existing Shader Program using its hash.
		// Returns 'nullptr' if not found
		std::shared_ptr<ShaderProgram> get_shaderprogram(const size_t hash) noexcept;

		// void cleanup() noexcept;
	private:
		// NOTE: VertexLayout object is stores on Assets
		// NOTE: With Pipeline Program there is no need to store Shader object
		std::unordered_map<size_t, std::weak_ptr<ShaderProgram>> shaderprogram_cache;
		// Helper to get obj from maps
		template <typename T>
		std::shared_ptr<T> get_obj(std::unordered_map<size_t, std::weak_ptr<T>>& map, const size_t hash) noexcept;
};


template <typename T>
std::shared_ptr<T> BufferManager::get_obj(std::unordered_map<size_t, std::weak_ptr<T>>& map, const size_t hash) noexcept {
	// Check if T is cached
	auto it = map.find(hash);
	if(it == map.end()) {
		return nullptr;
	}

	// If cached and valid
	if(std::shared_ptr<T> cache = it->second.lock()) {
		return cache;
	}

	// weak_ptr expired
	map.erase(it);
	return nullptr;
}


} // namespace floyd
