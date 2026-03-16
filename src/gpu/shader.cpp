#include <floydia/gpu/shader.hpp>

#include <stdexcept>
#include <floydia/utilities/string.hpp>

namespace floyd {

Shader::Shader(const char* source, const Shader::Type type) {
	if(source == nullptr) {
		throw std::invalid_argument("Shader source is null");
	}

	// Compiler shader
	if((this->shader = glCreateShader(static_cast<int>(type))) == 0) {
		throw std::runtime_error("Failed to compile shader");
	}
	glShaderSource(this->shader, 1, &source, NULL);
	glCompileShader(this->shader);

	GLint success;
	glGetShaderiv(this->shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		char log[1024];
		glGetShaderInfoLog(this->shader, 1024, NULL, log);
		glDeleteShader(this->shader); // Delete failed shader
		throw std::runtime_error(
			string::format("Error compiling %s shader:\n---\n%s\n---\n",
				(type == Shader::Type::Vertex) ? "VERTEX" : "FRAGMENT",
				log
			)
		);
	}
}

Shader::~Shader() noexcept {
	glDeleteShader(this->shader);
}

}
