#pragma once

#include <floydia/types.hpp>
#include <floydia/gpu/opengl.hpp>

namespace floyd {

// Represents a single shader inside a Shader Program
class Shader {
	public:
		enum class Type {
			None = 0,
			Vertex = GL_VERTEX_SHADER,
			Fragment = GL_FRAGMENT_SHADER
		};

		inline GLuint get_id() const noexcept { return this->id; }
		inline Shader::Type get_type() const noexcept { return this->type; }

		Shader(const char* source, const Shader::Type type);
		~Shader() noexcept;

	private:
		Shader::Type type = Shader::Type::None;
		GLuint id = 0;
};


} // namespace floyd
