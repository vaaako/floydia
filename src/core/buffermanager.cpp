#include <floydia/core/buffermanager.hpp>

#include <stdexcept>
#include <floydia/helpers/hash.hpp>
#include <floydia/helpers/logger.hpp>

namespace floyd {

std::shared_ptr<ShaderProgram> BufferManager::load_shaderprogram(const char* vertex, const char* fragment) {
	if(vertex == nullptr && fragment == nullptr) {
		throw std::invalid_argument("Vertex and Fragment Shader source are null");
	}

	// TODO: format shader if custom goes here

	size_t program_hash = 0;
	if(vertex != nullptr) {
		hash::combine(program_hash, hash::make(std::string_view(vertex)));
	}
	if(fragment != nullptr){
		hash::combine(program_hash, hash::make(std::string_view(fragment)));
	}

	// Check if program is cached
	std::shared_ptr<ShaderProgram> program = this->get_shaderprogram(program_hash);
	if(program != nullptr) {
		return program;
	}

	// Create program
	program = std::make_shared<ShaderProgram>();
	program->set_separable(true); // Pipeline by default
	if(vertex != nullptr) {
		Shader vs = Shader(vertex, Shader::Vertex);
		program->attach(vs);
	}
	if(fragment != nullptr) {
		Shader fs = Shader(fragment, Shader::Fragment);
		program->attach(fs);
	}
	program->link();

	program->hash = program_hash;
	this->shaderprogram_cache[program_hash] = program;
	return program;
}

std::shared_ptr<ShaderProgram> BufferManager::get_shaderprogram(const size_t hash) noexcept {
	return this->get_obj(this->shaderprogram_cache, hash);
}

} // namespace floyd
