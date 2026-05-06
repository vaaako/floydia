#include <floydia/gpu/programpipeline.hpp>
#include <floydia/helpers/logger.hpp>
#include <stdexcept>

namespace floyd {

ProgramPipeline::ProgramPipeline() {
	glCreateProgramPipelines(1, &this->pipeline);
	if(this->pipeline == 0) {
		throw std::runtime_error("Failed to create Program Pipeline");
	}
}

ProgramPipeline::~ProgramPipeline() noexcept {
	glDeleteProgramPipelines(1, &this->pipeline);
}

void ProgramPipeline::attach(const std::shared_ptr<ShaderProgram>& program, const Shader::Type type) const {
	glUseProgramStages(this->pipeline,
		(type == Shader::Vertex) ? GL_VERTEX_SHADER_BIT : GL_FRAGMENT_SHADER_BIT,
		program->id()
	);
}

void ProgramPipeline::validate() const {
	// Validate pipeline (optional but useful)
	glValidateProgramPipeline(this->pipeline);
	GLint valid = 0;
	glGetProgramPipelineiv(this->pipeline, GL_VALIDATE_STATUS, &valid);
	if(!valid) {
		char log[1024];
		glGetProgramPipelineInfoLog(this->pipeline, sizeof(log), NULL, log);
		TRACELOG(logger::Error, "Pipeline validation error:\n%s\n", log);
	}
}

}
