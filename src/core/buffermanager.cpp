#include <floydia/core/buffermanager.hpp>

#include <stdexcept>
#include <floydia/utilities/hash.hpp>

namespace floyd {

std::shared_ptr<ShaderProgram> BufferManager::load_shaderprogram(const char* vertex, const char* fragment) {
	if(vertex == nullptr) {
		throw std::invalid_argument("Vertex Shader source is null");
	}

	if(fragment == nullptr) {
		throw std::invalid_argument("Fragment Shader source is null");
	}

	// TODO: format shader if custom goes here

	size_t shaders_hash = 0;
	hash::combine(shaders_hash, hash::make(std::string_view(vertex)));
	hash::combine(shaders_hash, hash::make(std::string_view(fragment)));

	std::shared_ptr<Shader> vs = this->load_shader(vertex, Shader::Type::Vertex);
	std::shared_ptr<Shader> fs = this->load_shader(fragment, Shader::Type::Fragment);

	// Check if program is cached
	std::shared_ptr<ShaderProgram> program = this->get_shaderprogram(shaders_hash);
	if(program != nullptr) {
		return program;
	}

	// Create program
	program = std::make_shared<ShaderProgram>(vs, fs);
	// program->hash = shaders_hash;
	this->shaderprogram_cache[shaders_hash] = program;
	return program;
}

std::shared_ptr<Shader> BufferManager::load_shader(const char* source, const Shader::Type type) {
	const size_t hash = hash::make(std::string_view(source));

	std::shared_ptr<Shader> shader = this->get_shader(hash);
	if(shader != nullptr) {
		return shader;
	}

	// Compile shader
	shader = std::make_shared<Shader>(source, type);
	this->shader_cache[hash] = shader;
	return shader;
}

std::shared_ptr<ShaderProgram> BufferManager::get_shaderprogram(const size_t hash) noexcept {
	// Check if Shader is cached
	auto it = this->shaderprogram_cache.find(hash);
	if(it == this->shaderprogram_cache.end()) {
		return nullptr;
	}

	if(std::shared_ptr<ShaderProgram> cache = it->second.lock()) {
		return cache;
	}

	// weak_ptr expired
	this->shaderprogram_cache.erase(it);
	return nullptr;
}

std::shared_ptr<Shader> BufferManager::get_shader(const size_t hash) noexcept {
	// Check if Shader is cached
	auto it = this->shader_cache.find(hash);
	if(it == this->shader_cache.end()) {
		return nullptr;
	}

	if(std::shared_ptr<Shader> cache = it->second.lock()) {
		return cache;
	}

	// weak_ptr expired
	this->shader_cache.erase(it);
	return nullptr;
}

} // namespace floyd
