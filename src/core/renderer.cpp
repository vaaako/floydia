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
	this->batches.clear();
	this->instances.clear();
	// Update Camera and Camera Uniform Buffer
	CameraData cam = { camera.view(), camera.projection() };
	this->ubo_camera.update(&cam, sizeof(CameraData));
}

void Renderer::push(const Renderable& obj) noexcept {
	const glm::mat4& mmatrix = obj.transform.model_matrix(); // cache

	this->instances.push_back({ mmatrix, obj.color_norm() });
	// size() returns count, so -1 gives index of the last pushed element
	uint32 instance_index = this->instances.size() - 1;
	// Create draw commands with instance index
	for(const Model::SubMesh& sub : obj.model()->meshes()) {
		this->batches.push_back({
			sub.mesh.get(),
			sub.material.get(),
			1, // Count 1 for now
			instance_index // Start at this instance index
		});
	}
}

void Renderer::flush() {
	// Skip if queue is empty
	if(this->batches.empty()) {
		return;
	}

	// Sort by material then mesh
	// Before: ABACBA
	// After: AAABBC
	std::sort(this->batches.begin(), this->batches.end(),
	[](const DrawBatch& a, const DrawBatch& b) {
		if(a.material != b.material) {
			return a.material < b.material;
		}
		return a.mesh < b.mesh;
	});

	// Merge consecutive batches with same mesh/material
	std::vector<DrawBatch> merged;
	std::vector<InstanceData> new_instances; // reordered SSBO data
	merged.reserve(this->batches.size());
	new_instances.reserve(this->batches.size());
	// Group draw calls and rearrange instance data so each group is contigous
	// Before: AAABBC
	// After merged: A(3), B(2), C(1)
	for(const DrawBatch& batch : this->batches) {
		if(!merged.empty()
			&& merged.back().mesh == batch.mesh
			&& merged.back().material == batch.material) {
			// Same batch, extend instance count
			// Increase how many instances this batch draws
			merged.back().instance_count++;
			// Append instance next to previous ones
			new_instances.push_back(this->instances[batch.instance_index]);
			continue;
		}

		// Start a new batch
		DrawBatch new_batch = batch;
		// Mark start offset of this batch in the reordered instance buffer
		new_batch.instance_index = new_instances.size();
		new_batch.instance_count = 1;

		merged.push_back(new_batch);
		// Append instance next to previous ones
		new_instances.push_back(this->instances[batch.instance_index]);
	}

	// Update SSBO with all instance data
	this->instances.swap(new_instances);
	this->ssbo_instance.update(this->instances.data(),
			this->instances.size() * sizeof(InstanceData));

	// Draw merged batches
	Material* prev_material = nullptr;
	Mesh* prev_mesh = nullptr;
	for(const DrawBatch& batch : merged) {
		if(batch.material != prev_material) {
			batch.material->bind();
			// bind texture
			prev_material = batch.material;
		}

		if(batch.mesh != prev_mesh) {
			glBindVertexArray(batch.mesh->vaoid());
			prev_mesh = batch.mesh;
		}

		// Draw N instances starting from offset X in the instance buffer
		glDrawElementsInstancedBaseInstance(
			GL_TRIANGLES,
			batch.mesh->index_count,
			batch.mesh->index_type,
			NULL, // indices
			batch.instance_count,
			batch.instance_index // Base instance for gl_InstanceID
		);
	}
}

} // namespace floyd
