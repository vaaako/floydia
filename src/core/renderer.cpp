#include "floydia/gpu/shader.hpp"
#include <floydia/core/renderer.hpp>

#include <floydia/gpu/programpipeline.hpp>

#include <floydia/helpers/opengl.hpp>
#include <floydia/helpers/logger.hpp>

/*
- 5000 Cubes
- 5000 Planes
- No culling face
- Around 30s test for each
Shader Program: ~224-285 fps
Program Pipeline: ~257-303 fps
*/

// #define FLOYD_DEBUG_RENDERER

// TODO:
// - Currently InstanceData is being duplicated
//   + For batch and on 'instances'
// - MultiDrawIndirect?
// - Somehow cache Instances and only change when needed

namespace floyd {

Renderer::Renderer() noexcept :
	ubo_camera(0, sizeof(CameraData)),
	ssbo_instance(1, sizeof(InstanceData) * 100),
	ppipeline(ProgramPipeline())
	{

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// glEnable(GL_CULL_FACE);

	this->ppipeline.bind();
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
	// Advance frame
	this->frameindex = (this->frameindex + 1) % PersistentMappedBuffer::FRAMES_IN_FLIGHT; // Cap to ring buffer size
	// Clear previous frame
	this->batches.clear();
	this->instances.clear();
	this->total_instances = 0;
	// Update Camera and Camera Uniform Buffer
	const glm::mat4 view = camera.view();
	const glm::mat4 proj = camera.projection();
	CameraData cam = { view, proj };
	const size_t offset = this->ubo_camera.frame_offset(frameindex);
	this->ubo_camera.update(&cam, sizeof(CameraData), offset);
	this->ubo_camera.flush(offset, sizeof(CameraData)); // Bind for shader use
	// Update frustum once per frame
	this->frustum.update(proj * view);
}

void Renderer::push(Renderable& obj) noexcept {
	// Needed for Frustum Culling
	if(obj.transform.isdirty()) {
		obj.rebuild_world_aabb();
	}
	if(!this->frustum.test(obj.world_aabb)) return;

	const glm::mat4& mmatrix = obj.transform.model_matrix(); // this updates model matrix
	InstanceData data = { mmatrix, obj.color_norm() };
	// Create draw commands with instance index
	for(const Model::SubMesh& sub : obj.model()->meshes()) {
		// Cull entire object against Model AABB
		// if(!this->frustum.test(sub.mesh->aabb, mmatrix)) continue;

		// Try to find existing batch
		BatchKey key = { sub.mesh.get(), sub.material.get() };
		auto it = this->batches.find(key);

		// Existing batch
		if(it != this->batches.end()) {
			++it->second.instance_count;
			it->second.instances.push_back(data);

		// Not found. Make new batch
		} else {
		#if defined(FLOYD_DEBUG_RENDERER)
			TRACELOG(logger::Debug, "Pushing NEW batch. Instance Index: %d", this->instances.size());
		#endif

			DrawBatch batch;
			batch.mesh = key.mesh;
			batch.material = key.material;
			batch.instance_count = 1;
			batch.instances.push_back(data);

			this->batches.emplace(key, std::move(batch));
		}

		// NOTE:
		// If 'instance_indices' were appended here
		// the layout would follow this order:
		// - Mesh A: [A0]
		// - Mesh B: [A0, B1]
		// - Mesh A: [A0, B1, A2]
		// The GPU requires per-batch contiguous ranges
		// Pushing indices inside 'flush' becomes:
		// - Batch A: [A0, A2]
		// - Batch B: [B1]
		++total_instances;
	}
}

void Renderer::flush() noexcept {
	// Skip if queue is empty
	if(this->batches.empty()) {
		return;
	}

	// Store intance index inside its batch
	this->instances.reserve(this->total_instances);
	for(auto& [_, batch] : this->batches) {
		batch.instance_index = this->instances.size();
		// Bulk copy. Insert all of batch's instances into 'instances'
		this->instances.insert(
			this->instances.end(),
			batch.instances.begin(),
			batch.instances.end()
		);
	}

	// Update SSBO with all instance data
	// Resize if necessary. Grow with margin
	this->ssbo_instance.resize(this->total_instances * sizeof(InstanceData));
	const size_t size = this->instances.size() * sizeof(InstanceData);
	const size_t offset = this->ssbo_instance.frame_offset(frameindex);
	this->ssbo_instance.update(this->instances.data(), size, offset);
	this->ssbo_instance.flush(offset, size);

#if defined(FLOYD_DEBUG_RENDERER)
	const float avg_instances = (float)this->instances.size() / this->batches.size();
	TRACELOG(logger::Debug, "Avg instances per batch: %.2f", avg_instances);
	TRACELOG(logger::Debug, "Draw calls: %zu -> %zu", this->instances.size(), this->batches.size());
#endif

	// Draw merged batches
	ShaderProgram* prev_vertex = nullptr;
	ShaderProgram* prev_fragment = nullptr;
	// Material* prev_material = nullptr;
	Mesh* prev_mesh = nullptr;

	for(const auto& [key, batch] : this->batches) {
		// pipeline stage swaps
		if(prev_vertex != batch.material->vertex.get()) {
			this->ppipeline.attach(batch.material->vertex, Shader::Vertex);
			prev_vertex = batch.material->vertex.get();
		}
		if(prev_fragment != batch.material->fragment.get()) {
			this->ppipeline.attach(batch.material->fragment, Shader::Fragment);
			prev_fragment = batch.material->fragment.get();
		}

		// surface data
		// if(batch.material != prev_material) {
		// 	// batch.material->bind();
		// 	batch.material->vertex->bind();
		// 	prev_material = batch.material;
		// }

		if(batch.mesh != prev_mesh) {
			glBindVertexArray(batch.mesh->vaoid());
			prev_mesh = batch.mesh;
		}

		// Draw N instances starting from offset X in the instance buffer
		glDrawElementsInstancedBaseInstance(
			GL_TRIANGLES,
			batch.mesh->index_count,
			batch.mesh->index_type,
			(void*)0, // indices
			batch.instance_count,
			batch.instance_index // gl_BaseInstance
		);
	}
}

} // namespace floyd
