#include <floydia/gpu/shaderprogram.hpp>

#include <stdexcept>
#include <floydia/utilities/string.hpp>

namespace floyd {

ShaderProgram::ShaderProgram(const std::shared_ptr<Shader>& v_shader, const std::shared_ptr<Shader>& f_shader) {
	// Validate shader
	if(v_shader == nullptr || v_shader->get_id() == 0) {
		throw std::invalid_argument("Vertex Shader is invalid");
	}

	if(f_shader == nullptr || f_shader->get_id() == 0) {
		throw std::invalid_argument("Fragment Shader is invalid");
	}

	// Cache
	GLuint vs = v_shader->get_id();
	GLuint fs = f_shader->get_id();

	// Create program
	if((this->program = glCreateProgram()) == 0) {
		throw std::runtime_error("Failed to compile Shader Program");
	}

	glAttachShader(this->program, vs);
	glAttachShader(this->program, fs);
	glLinkProgram(this->program);

	// Detach used shaders
	glDetachShader(this->program, vs);
	glDetachShader(this->program, fs);

	GLint success;
	glGetProgramiv(this->program, GL_LINK_STATUS, &success);
	if(!success) {
		GLchar log[1024];
		glGetProgramInfoLog(this->program, 1024, NULL, log);
		glDeleteShader(this->program); // Clean up failed program
		throw std::runtime_error(
			string::format("Error compiling Shader Program:\n---\n%s\n---\n", log)
		);
	}
}

ShaderProgram::~ShaderProgram() noexcept {
	glDeleteProgram(this->program);
}


GLint ShaderProgram::get_uniform_loc(const std::string& name) noexcept {
	if(this->uniforms_cache.find(name) != this->uniforms_cache.end()) {
		return this->uniforms_cache[name];
	}
	GLint location = glGetUniformLocation(this->program, name.c_str());
	this->uniforms_cache[name] = location;
	return location;
}

} // namespace floyd
