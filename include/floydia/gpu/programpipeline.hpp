#pragma once

#include <floydia/gpu/shader.hpp>
#include <floydia/gpu/shaderprogram.hpp>

namespace floyd {

class ProgramPipeline final {
	public:
		ProgramPipeline();
		~ProgramPipeline() noexcept;

		inline void bind() const noexcept {
			glBindProgramPipeline(this->pipeline);
		}

		inline u32 id() const noexcept { return this->pipeline; }

		// Attach a program to Program Pipeline
		void attach(const u32 program_id, const Shader::Type type) const;
		// Optional. Make sure attachments are ok
		void validate() const;

	private:
		GLuint pipeline;
};

}
