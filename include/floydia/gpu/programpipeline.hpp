#pragma once

#include <floydia/gpu/shader.hpp>
#include <floydia/gpu/shaderprogram.hpp>
#include <memory>

namespace floyd {

class ProgramPipeline final {
	public:
		ProgramPipeline();
		~ProgramPipeline() noexcept;

		inline void bind() const noexcept {
			glBindProgramPipeline(this->pipeline);
		}

		inline GLuint get_id() const noexcept { return this->pipeline; }

		// Attach a program to Program Pipeline
		void attach(const std::shared_ptr<ShaderProgram>& program, const Shader::Type type) const;
		// Optional. Make sure attachments are ok
		void validate() const;

	private:
		GLuint pipeline;
};

}
