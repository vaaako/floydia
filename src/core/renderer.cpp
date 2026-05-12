#include <floydia/core/core.hpp>
#include <floydia/gpu/shader.hpp>
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

namespace floyd {

Renderer::Renderer() noexcept :
	ubo_camera(0, sizeof(CameraData)),
	ssbo_objs(1, sizeof(InstanceData) * 100),
	ssbo_lights(2, sizeof(Light::LightData) * 10),
	ppipeline(ProgramPipeline())
	{

	// glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	// Block CPU until GPU is done reading this frame's buffer slots.
	this->ubo_camera.wait(this->frameindex); // Correct but overkill
	this->ssbo_objs.wait(this->frameindex);
	this->ssbo_lights.wait(this->frameindex);

	// Clear previous frame
	this->dynamic_batches.clear();
	this->instances.clear();
	this->lights.clear();
	this->total_instances = 0;

	// Update Camera and Camera Uniform Buffer
	this->cached_view = camera.view();
	this->cached_proj = camera.projection();
	this->camerapos = camera.position;

	// Update frustum once per frame
	glm::mat4 vp = this->cached_proj * this->cached_view;
	this->frustum.update(vp);
	this->camera_dirty = this->camera_moved(vp);

	// Rebuild persistent batch cache only when dirty
	if(this->persistent_dirty) {
		this->persistent_batches.clear();
		for(const Renderable* obj : this->persistent_objs) {
			if(!obj) continue;
			this->add_batch(*obj, this->persistent_batches);
		}
		this->persistent_dirty = false;
	}

	// Update persistent batches
	for(auto& [_, batch] : this->persistent_batches) {
		batch.instance_index = this->instances.size();
		// Bulk copy. Insert all of batch's instances into 'instances'
		this->instances.insert(
			this->instances.end(),
			batch.instances.begin(),
			batch.instances.end()
		);
	}

	// Rebuild persistent lights
	for(const Light* light : this->persistent_lights) {
		this->lights.push_back(light->to_gpu_data());
	}
}

void Renderer::push(Renderable& obj) noexcept {
	// Only cull if obj changed or camera moved
	if(obj.transform.isdirty()) {
		obj.rebuild_world_aabb();
		obj.visible = this->frustum.test(obj.world_aabb);
	} else if(this->camera_dirty) {
		obj.visible = this->frustum.test(obj.world_aabb);
	}
	if(!obj.visible) return;
	this->add_batch(obj, this->dynamic_batches);
}

size_t Renderer::add(const Renderable& obj) noexcept {
	this->persistent_objs.push_back(&obj);
	this->persistent_dirty = true;
	this->persistent_ssbo_objs_dirty = PersistentMappedBuffer::FRAMES_IN_FLIGHT; // upload to all frame slots
	return this->persistent_objs.size() - 1;
}

void Renderer::push(const Light& light) noexcept {
	this->lights.push_back(light.to_gpu_data());
}

size_t Renderer::add(const Light& light) noexcept {
	this->persistent_lights.push_back(&light);
	this->persistent_ssbo_light_dirty = PersistentMappedBuffer::FRAMES_IN_FLIGHT; // upload to all frame slots
	return this->persistent_lights.size() - 1;
}

void Renderer::add_batch(const Renderable& obj, std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& target) noexcept {
	// NOTE: Culling for both types of objects may cause problems.
	// If Persistent Object was added not visible, it is likely to never be visible
	// (excepts a new object is added when it is visible)
	const glm::mat4& mmatrix = obj.transform.model_matrix(); // this updates model matrix
	InstanceData data = { mmatrix, obj.color_norm() };
	// Create draw commands with instance index
	for(const Model::SubMesh& sub : obj.model()->meshes()) {
		// Cull sub meshes
		// if(!this->frustum.test(sub.mesh->aabb, mmatrix)) continue;

		// Try to find existing batch
		BatchKey key = {
			sub.mesh.get(),
			sub.material->base->vertex->id(),
			sub.material->base->fragment->id(),
			(sub.material->albedo) ? sub.material->albedo->id() : 0
		};

		auto [it, emplaced] = target.try_emplace(key);
		// 'emplaced' is false if collided (batch exists)
		DrawBatch& batch = it->second;
		if(emplaced) {
		#if defined(FLOYD_DEBUG_RENDERER)
			TRACELOG(logger::Debug, "Pushing NEW batch. Instance Index: %d", this->instances.size());
		#endif
			batch.mesh = sub.mesh.get();
			batch.matinst = sub.material.get();
			// batch.instance_count = 1;
			// batch.instances.push_back(data);
		}
		batch.instance_count++;
		batch.instances.push_back(data);
		++total_instances;

		// NOTE: If 'instances' were appended here
		// the layout would follow this order:
		// - Mesh A: [A0]
		// - Mesh B: [A0, B1]
		// - Mesh A: [A0, B1, A2]
		//
		// The GPU requires per-batch contiguous ranges.
		// Pushing indices inside 'flush' becomes:
		// - Batch A: [A0, A2]
		// - Batch B: [B1]
	}
}

void Renderer::flush() noexcept {
	// Skip if queue is empty
	if(this->dynamic_batches.empty() && this->persistent_batches.empty()) return;

	// Update camera here so 'lights' is updated
	CameraData cam = { this->cached_view, this->cached_proj, glm::vec4(this->camerapos, 1.0f) };
	const size_t offset = this->ubo_camera.frame_offset(frameindex);
	this->ubo_camera.update(&cam, sizeof(CameraData), offset);
	this->ubo_camera.flush(offset, sizeof(CameraData)); // Bind for shader use

	// Update dynamic batches
	for(auto& [_, batch] : this->dynamic_batches) {
		batch.instance_index = this->instances.size();
		// Bulk copy. Insert all of batch's instances into 'instances'
		this->instances.insert(
			this->instances.end(),
			batch.instances.begin(),
			batch.instances.end()
		);
	}

#if defined(FLOYD_DEBUG_RENDERER)
	const size_t total_batches   = this->dynamic_batches.size() + this->persistent_batches.size();
	const size_t total_instances = this->instances.size();
	const float avg = (float)total_instances / total_batches;
	TRACELOG(logger::Debug, "Avg instances per batch: %.2f", avg);
	TRACELOG(logger::Debug, "Draw calls: %zu -> %zu", total_instances, total_batches);
#endif

	// Update SSBO when there are dynamic objects or persistent slots need filling
	if(!this->instances.empty() || this->persistent_ssbo_objs_dirty > 0) {
		// Resize if necessary. Grow with margin
		const size_t size = this->instances.size() * sizeof(InstanceData);
		if(size > this->ssbo_objs.perframesize()) this->ssbo_objs.resize(size);

		const size_t offset = this->ssbo_objs.frame_offset(this->frameindex);
		this->ssbo_objs.update(this->instances.data(), size, offset);
		this->ssbo_objs.flush(offset, size);

		// Persistent objects are added once, so it is needed to manually
		// force upload for each FRAMES_IN_FLIGHT slot. Decrement until all slots are filled
		if(this->persistent_ssbo_objs_dirty > 0) --this->persistent_ssbo_objs_dirty;
	}

	// Update lights on scene
	if(!this->lights.empty() || this->persistent_ssbo_light_dirty > 0) {
		if(!this->lights.empty()) {
			Light::LightBuffer header;
			header.count = static_cast<uint32>(this->lights.size());
			const size_t header_size = sizeof(Light::LightBuffer);
			const size_t data_size = this->lights.size() * sizeof(Light::LightData);

			const size_t total_size = header_size + data_size;
			if(total_size > this->ssbo_lights.perframesize()) this->ssbo_lights.resize(total_size);

			const size_t offset = this->ssbo_lights.frame_offset(this->frameindex);
			this->ssbo_lights.update(&header, header_size, offset);
			this->ssbo_lights.update(this->lights.data(), data_size, offset + header_size); // Append after header
			this->ssbo_lights.flush(offset, total_size);
		}
		if(this->persistent_ssbo_light_dirty > 0) --this->persistent_ssbo_light_dirty;
	}

	// Render all objects
	this->draw_map(this->persistent_batches);
	this->draw_map(this->dynamic_batches);

	// Signal GPU fence, marks this frame's buffer slots as in-flight
	this->ubo_camera.lock(this->frameindex);
	this->ssbo_objs.lock(this->frameindex);
	this->ssbo_lights.lock(this->frameindex);
}


void Renderer::upload_lights() noexcept {
	for(const Light* l : this->static_lights) {
		this->lights.push_back(l->to_gpu_data());
	}

	for(const Light* l : this->dynamic_lights) {
		this->lights.push_back(l->to_gpu_data());
	}
}

void Renderer::draw_batch(const std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& batchmap) const noexcept {
	Mesh* prev_mesh = nullptr;
	ShaderProgram* prev_vertex = nullptr;
	ShaderProgram* prev_fragment = nullptr;
	MaterialInstance* prev_material = nullptr;

	for(const auto& [_, batch] : batchmap) {
		if(batch.mesh != prev_mesh) {
			glBindVertexArray(batch.mesh->vaoid());
			prev_mesh = batch.mesh;
		}

		if(prev_material != batch.matinst) {
			// Material& mat = *batch.material->base; // cache
			Material& mat = *batch.matinst->base; // cache
			// pipeline stage swaps
			if(prev_vertex != mat.vertex.get()) {
				this->ppipeline.attach(mat.vertex, Shader::Vertex);
				prev_vertex = mat.vertex.get();
			}
			if(prev_fragment != mat.fragment.get()) {
				this->ppipeline.attach(mat.fragment, Shader::Fragment);
				prev_fragment = mat.fragment.get();
			}
			batch.matinst->bind();
			prev_material = batch.matinst;
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


Cube Renderer::show_light(const Light& light) noexcept {
	Assets* assets = Core::get().assets.get();

	Cube cube = Cube();
	cube.transform.set_position(light.transform.position());
	cube.transform.set_scale({0.2f, 0.2f, 0.2f});
	cube.transform.set_rotation(light.transform.rotation());
	cube.set_color(light.color());
	// Set to be not affected by light
	cube.material()->base =
		assets->load_material(
			assets->load_program(Shaders::DEFAULT_VERTEX, nullptr),
			assets->load_program(nullptr, Shaders::DEFAULT_FRAGMENT_UNLIT)
		);
	this->add(cube);
	return cube;
}

} // namespace floyd
