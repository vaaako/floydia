#include <floydia/core/renderer.hpp>

#include <algorithm>
#include <floydia/utilities/log.hpp>

namespace floyd {

	// TODO: this is fixeed to 1000 instances
	// add dynamic resizing later
Renderer::Renderer() noexcept
	: ubo_camera(sizeof(CameraData)),
	ssbo_instance(sizeof(InstanceData) * Renderer::INST_AMOUNT) {} // 1000

void Renderer::init() noexcept {
	ubo_camera.init(0);
	ssbo_instance.init(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

void Renderer::clear() const noexcept {
	glClearColor(this->clear_color[0], this->clear_color[1], this->clear_color[2], this->clear_color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::set_clear_color(const vec4<uint8>& color) noexcept {
	this->clear_color[0] = color.x / 255.0f;
	this->clear_color[1] = color.y / 255.0f;
	this->clear_color[2] = color.z / 255.0f;
	this->clear_color[3] = color.w / 255.0f;
}

void Renderer::begin_draw(const Camera& camera) noexcept {
	// Clear previous frame
	this->draw_queue.clear();
	this->instances.clear();
	// Update Camera and Camera Uniform Buffer
	CameraData cam = { camera.view(), camera.projection() };
	this->ubo_camera.update(&cam, sizeof(CameraData));
}

void Renderer::push(const Renderable& obj) noexcept {
	const glm::mat4& mmatrix = obj.transform.model_matrix(); // cache
	this->draw_queue.reserve(this->draw_queue.size() + obj.mesh_count());
	for(const Model::SubMesh& sub : obj.model()->meshes()) {
		this->draw_queue.push_back({
			sub.mesh.get(),
			sub.material.get(),
			mmatrix
		});
	}
	this->instances.push_back({ mmatrix });
}

// TODO: sort instances
void Renderer::flush() {
	// Skip if queue is empty
	if(this->draw_queue.empty()) {
		return;
	}

	// Group similar meshes
	std::sort(this->draw_queue.begin(), this->draw_queue.end(),
	[](const DrawCommand& a, const DrawCommand& b) {
		if(a.material != b.material) {
			return a.material < b.material;
		}
		return a.mesh < b.mesh;
	});

	// Update SSBO with all instance data
	this->instances.resize(this->draw_queue.size());
	this->ssbo_instance.update(this->instances.data(),
			this->instances.size() * sizeof(InstanceData));

	// TODO: cache shader, texture and vao

	Material* prev_material = nullptr;
	Mesh* prev_mesh = nullptr;

	// Draw in batches
	const size_t amount = this->draw_queue.size();
	for(const DrawCommand& cmd : this->draw_queue) {
		if(cmd.material != prev_material) {
			cmd.material->bind();
			// bind texture
			prev_material = cmd.material;
		}

		if(cmd.mesh != prev_mesh) {
			glBindVertexArray(cmd.mesh->vaoid());
			prev_mesh = cmd.mesh;
		}

		glDrawElementsInstanced(
			GL_TRIANGLES,
			cmd.mesh->index_count,
			cmd.mesh->index_type,
			NULL, // indices
			amount // instance count for this mesh
		);
	}
}

} // namespace floyd
