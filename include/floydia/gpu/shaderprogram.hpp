#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <floydia/gpu/shader.hpp>
#include <floydia/gpu/opengl.hpp>

#include <glm/ext.hpp> // glm::value_ptr

namespace floyd {

// Shader Program Object
class ShaderProgram {
	public:
		ShaderProgram(const std::shared_ptr<Shader>& v_shader, const std::shared_ptr<Shader>& f_shader);
		~ShaderProgram() noexcept;

		inline void bind() const noexcept {
			glUseProgram(this->program);
		}

		inline void unbind() const noexcept {
			glUseProgram(0);
		}

		inline GLuint get_id() const noexcept { return this->program; };
		// Returns '0' if this class was not created by BufferManager
		// inline GLuint get_hash() const noexcept { return this->hash; };

		inline void set_uniform_int(const char* name, const int value) noexcept {
			glUniform1f(this->get_uniform_loc(std::string(name)), value);
		}

		inline void set_uniform_vec4f(const char* name, const glm::vec4& value) noexcept {
			glUniform4f(this->get_uniform_loc(std::string(name)), value.x, value.y, value.z, value.w);
		}

		inline void set_uniform_mat4f(const char* name, const glm::mat4& value, const GLsizei index = 1) noexcept {
			glUniformMatrix4fv(this->get_uniform_loc(std::string(name)), index, GL_FALSE, glm::value_ptr(value));
		}

	private:
		std::unordered_map<std::string, GLint> uniforms_cache;
		// NOTE: Storing as 'shared_ptr<Shader>' so the weak pointer does not get expired
		std::shared_ptr<Shader> v_shader;
		std::shared_ptr<Shader> f_shader;
		GLuint program;

		GLint get_uniform_loc(const std::string& name) noexcept;
};

} // namespace floyd
