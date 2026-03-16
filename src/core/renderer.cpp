#include <floydia/core/renderer.hpp>
#include <floydia/gfx/renderable.hpp>

#include <floydia/utilities/log.hpp>

namespace floyd {

Renderer::Renderer() noexcept :
	ubo_camera(0, sizeof(CameraData)), ssbo_instance(1, sizeof(InstanceData) * 1000) {
	// TODO: this is fixeed to 1000 instances
	// add dynamic resizing later
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
}

void Renderer::begin_frame(Camera& camera) noexcept {
	this->draw_queue.clear();
	this->instances.clear();

	CameraData cam = { camera.view(), camera.projection() };
	this->ubo_camera.update(&cam, sizeof(CameraData));
}

void Renderer::submit(const DrawCommand& cmd) noexcept {
	this->draw_queue.push_back(cmd);
}

// TODO: sort instances
void Renderer::flush() {
	if(this->draw_queue.empty()) {
		TRACELOG(log::Type::Debug, "instances is empty. aborting.");
		return;
	}

	// Build instance data
	this->instances.reserve(this->draw_queue.size());
	for(const DrawCommand& cmd : this->draw_queue) {
		this->instances.push_back({ cmd.model });
	}

	this->ssbo_instance.update(this->instances.data(),
			this->instances.size() * sizeof(InstanceData));

	// Draw
	for(size_t i = 0; i < draw_queue.size(); i++) {
		const DrawCommand& cmd = this->draw_queue[i];
		cmd.material->bind(); // shader, color and texture (todo)

		glBindVertexArray(cmd.mesh->vaoid());
		// 'glDrawElementsInstanced' has the same effect as:
		// for-loop with glDrawElements
		glDrawElementsInstanced(
			GL_TRIANGLES,
			cmd.mesh->index_count,
			cmd.mesh->index_type,
			NULL, // indices
			1     // instance count
		);
	}
}

} // namespace floyd
