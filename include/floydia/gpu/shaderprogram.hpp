#pragma once

#include <memory>
#include <floydia/gpu/shader.hpp>
#include <floydia/gpu/opengl.hpp>

namespace floyd {

// Shader Program Object
class ShaderProgram {
	public:
		ShaderProgram(const std::shared_ptr<Shader>& v_shader, const std::shared_ptr<Shader>& f_shader);
		~ShaderProgram() noexcept;

		inline void use() const noexcept {
			glUseProgram(this->id);
		}

		inline void disable() const noexcept {
			glUseProgram(0);
		}

		inline GLuint get_id() const noexcept { return this->id; };
		// Returns '0' if this class was not created by BufferManager
		// inline GLuint get_hash() const noexcept { return this->hash; };

	private:
		// NOTE: Storing as 'shared_ptr<Shader>' so the weak pointer does not get expired
		std::shared_ptr<Shader> v_shader;
		std::shared_ptr<Shader> f_shader;
		// size_t hash = 0; // Defined by BufferManager
		GLuint id;
};

} // namespace floyd
