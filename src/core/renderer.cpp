#include <floydia/core/renderer.hpp>
#include <floydia/gfx/renderable.hpp>

namespace floyd {

void Renderer::submit(const DrawCommand& cmd) noexcept {
	this->draw_queue.push_back(cmd);
}

void Renderer::flush() {
	// sort_by_material_and_mesh(this->draw_queue)

	// TODO: draw here
	//	Before draw UI: glDisable(GL_DEPTH_TEST)
	this->draw_queue.clear();
}

} // namespace floyd
