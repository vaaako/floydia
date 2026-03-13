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
	if((this->id = glCreateProgram()) == 0) {
		throw std::runtime_error("Failed to compile Shader Program");
	}

	glAttachShader(this->id, vs);
	glAttachShader(this->id, fs);
	glLinkProgram(this->id);

	// Detach used shaders
	glDetachShader(this->id, vs);
	glDetachShader(this->id, fs);

	GLint success;
	glGetProgramiv(this->id, GL_LINK_STATUS, &success);
	if(!success) {
		GLchar log[1024];
		glGetProgramInfoLog(this->id, 1024, NULL, log);
		glDeleteShader(this->id); // Clean up failed program
		throw std::runtime_error(
			string::format("Error compiling Shader Program:\n---\n%s\n---\n", log)
		);
	}
}

ShaderProgram::~ShaderProgram() noexcept {
	glDeleteProgram(this->id);
}

} // namespace floyd
