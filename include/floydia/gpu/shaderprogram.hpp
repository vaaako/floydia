#pragma once

#include <string>
#include <unordered_map>
#include <floydia/gpu/opengl.hpp>
#include <floydia/gpu/shaders.hpp>
#include <floydia/gpu/shader.hpp>

#include <glm/ext.hpp> // glm::value_ptr

class BufferManager;

namespace floyd {

// Shader Program Object
class ShaderProgram {
	friend class BufferManager;

	public:
		ShaderProgram();
		~ShaderProgram() noexcept;

		// This does not check if program was linked
		inline void bind() const noexcept {
			glUseProgram(this->program);
		}

		inline void unbind() const noexcept {
			glUseProgram(0);
		}

		inline GLuint get_id() const noexcept { return this->program; };
		// Returns '0' if this class was not created by BufferManager
		inline size_t get_hash() const noexcept { return this->hash; };

		// Takes shader ownership and attach to program.
		// Does nothing if program already has a shader attached
		void attach(Shader& shader);
		// Set as true to use this program with Pipeline Program.
		// Set this before attaching shaders. Will have no effect if shader is already linked
		void set_separable(const bool value) noexcept;
		// Link attached shaders to program
		void link();

		inline void set_uniform_int(const char* name, const int value) noexcept {
			glProgramUniform1i(this->program, this->get_uniform_loc(std::string(name)), value);
		}

		inline void set_uniform_vec4f(const char* name, const glm::vec4& value) noexcept {
			glProgramUniform4f(this->program, this->get_uniform_loc(std::string(name)), value.x, value.y, value.z, value.w);
		}

		inline void set_uniform_mat4f(const char* name, const glm::mat4& value, const GLsizei index = 1) noexcept {
			glProgramUniformMatrix4fv(this->program, this->get_uniform_loc(std::string(name)), index, GL_FALSE, glm::value_ptr(value));
		}

	private:
		std::unordered_map<std::string, GLint> uniforms_cache;
		size_t hash = 0; // Set by BufferManager
		// NOTE: Storing as 'shared_ptr<Shader>' so the weak pointer does not get expired
		GLuint program;
		// TODO: currently supporting only these shaders.
		// Add geometry and compute later
		GLuint v_shader = 0;
		GLuint f_shader = 0;
		bool linked = false;
		bool separable = false;

		GLint get_uniform_loc(const std::string& name) noexcept;
};

} // namespace floyd
