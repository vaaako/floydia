#include <floydia/gpu/shaderprogram.hpp>

#include <stdexcept>
#include <floydia/utilities/string.hpp>
#include <floydia/utilities/log.hpp>

namespace floyd {

ShaderProgram::ShaderProgram() {
	if((this->program = glCreateProgram()) == 0) {
		throw std::runtime_error("Failed to compile Shader Program");
	}
}

ShaderProgram::~ShaderProgram() noexcept {
	glDeleteProgram(this->program);
}

void ShaderProgram::attach(Shader& shader) {
	if(this->linked) {
		TRACELOG(log::type::Error, "Cannot attach shader after linking");
		return;
	}

	// Validate shader
	GLuint id = shader.release();
	if(id == 0) {
		TRACELOG(log::type::Error, "Invalid or already-released Shader");
		return;
	}

	glAttachShader(this->program, id);
	if(shader.type == Shader::Vertex && this->v_shader == 0) {
		this->v_shader = id;

	} else if(shader.type == Shader::Fragment && this->f_shader == 0) {
		this->f_shader = id;
	}
}

void ShaderProgram::set_separable(const bool value) noexcept {
	if(this->linked) {
		TRACELOG(log::type::Error, "Program already linked");
		return;
	}
	glProgramParameteri(this->program, GL_PROGRAM_SEPARABLE, value);
	this->separable = true;
}

void ShaderProgram::link() {
	if(this->linked) {
		TRACELOG(log::type::Error, "Program already linked");
		return;
	}

	glLinkProgram(this->program);

	GLint success;
	glGetProgramiv(this->program, GL_LINK_STATUS, &success);
	if(!success) {
		GLchar log[1024];
		glGetProgramInfoLog(this->program, 1024, NULL, log);
		glDeleteProgram(this->program); // Clean up failed program
		this->program = 0; // prevent double-delete in destructor
		throw std::runtime_error(
			string::format("Error linking Shader Program:\n---\n%s\n---\n", log)
		);
	}

	// Delete used shaders
	if(this->v_shader != 0) {
		glDeleteShader(this->v_shader);
	}

	if(this->f_shader != 0) {
		glDeleteShader(this->f_shader);
	}

	this->linked = true;
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
