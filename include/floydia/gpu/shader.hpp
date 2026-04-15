#pragma once

#include <floydia/types.hpp>

#include <glad/gl.h>

namespace floyd {

// Represents a single shader inside a Shader Program
class Shader {
	public:
		enum Type {
			None = 0,
			Vertex   = GL_VERTEX_SHADER,
			Fragment = GL_FRAGMENT_SHADER
		};

		Shader::Type type = Shader::Type::None;

		Shader(const char* source, const Shader::Type type);
		~Shader() noexcept;

		inline GLuint get_id() const noexcept { return this->shader; }
		// Extracts Shader's Id and takes ownership
		GLuint release() noexcept;

	private:
		GLuint shader = 0;
};


} // namespace floyd
