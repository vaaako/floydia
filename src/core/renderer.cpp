#include "floydia/core/renderer.hpp"
#include "floydia/geometry/text.hpp"
#include "floydia/rendering/light.hpp"
#include "floydia/window/keycode.hpp"
#include <unordered_set>

#if defined(FLOYD_DEBUG_RENDERER)
#include "floydia/helpers/logger.hpp"
#endif

namespace floyd {

Renderer::Renderer() noexcept :
	ubo_camera(0, sizeof(Camera::CameraData) * 2),
	ssbo_objs(1, sizeof(Renderable::InstanceData) * 128),
	ssbo_lights(2, sizeof(Light::LightData) * 10),
	ssbo_glyphs(3, sizeof(Text::GlyphData) * 255),
	ppipeline(ProgramPipeline())
	{

	// Reserve right away
	// this->instances.reserve(128);
	this->static_instances.reserve(128);
	// this->lights.reserve(10);
	// this->glyphs.reserve(256);

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	this->ppipeline.bind();
}

Renderer::Batch& Renderer::resolve_batch(const Model::SubMesh& sub, BatchTable& table) noexcept {
	BatchKey key {
		sub.mesh.get(),
		sub.material.vertex->id(),
		sub.material.fragment->id(),
		sub.material.albedo ? sub.material.albedo->id() : 1
	};

	Batch& batch = table[key]; // Creates a new default Batch if new
	if(!batch.mesh) {
		// New batch
		batch.mesh = sub.mesh.get();
		batch.material = &sub.material;
	}
	return batch;
}

void Renderer::insert_static(Renderable& obj) noexcept {
	obj.world_aabb(); // If called after 'model_matrix' it would return with old Transform, because 'dirty' flag was consumed
	const glm::mat4& m = obj.transform.model_matrix();
	const vec4<float>& color = obj.color_norm();

	std::vector<SlotLocation>& slots = this->static_lookup[&obj]; // new entry
	for(const Model::SubMesh& sub : obj.model()->meshes()) {
		Batch& batch = this->resolve_batch(sub, this->dynamic_batches);
		u32 idx = batch.push_static({
			m, color, sub.material.metallic, sub.material.roughness,
		}, &obj);
		slots.push_back({ &batch, idx });
	}
}

void Renderer::insert_dynamic(Renderable& obj) noexcept {
	const glm::mat4& m = obj.transform.model_matrix();
	const vec4<float>& color = obj.color_norm();

	for(const Model::SubMesh& sub : obj.model()->meshes()) {
		Batch& batch = this->resolve_batch(sub, this->static_batches);
		batch.push_dynamic({
			m, color, sub.material.metallic, sub.material.roughness,
		});
	}
}

void Renderer::flatten_persistent() noexcept {
	this->static_batches.clear();
	this->static_lookup.clear();

	// Rebuild every batch
	for(Renderable* obj : this->static_objs) {
		this->insert_static(*obj);
	}

	// Sum instance count across all batches first, so 'static_instances' is reserved exactly
	size_t total = 0;
	for(auto& [key, batch] : this->static_batches) {
		total += batch.gpu_data.size();
	}

	this->static_instances.clear();
	this->static_instances.reserve(total);

	for(auto& [key, batch] : this->static_batches) {
		batch.instance_index = static_cast<u32>(this->static_instances.size());

		this->static_instances.insert(this->static_instances.end(),
			batch.gpu_data.begin(), batch.gpu_data.end());

		batch.aabb = AABB{};
		for(Renderable* owner : batch.owners) {
			batch.aabb.merge(owner->world_aabb());
		}

		// 'gpu_data' is only needed as scratch to build 'static_instances'
		batch.gpu_data.clear();
		batch.gpu_data.shrink_to_fit();
	}
}

void Renderer::patch_dirty() noexcept {
	// Batches that changed AABB
	std::unordered_set<Batch*> touched_batches;

	for(Renderable* obj : this->dirty_queue) {
		obj->is_dirty_queued = false;

		// Material/Mesh changed this object's BatchKey.
		// Can't patch in-place and its old slot no longer corresponds to the right batch.
		// Need to have a full rebuild next frame instead
		if(obj->needs_rebatch) {
			obj->needs_rebatch = false;
			this->static_dirty = true;
			continue;
		}

		const bool transform_changed = obj->transform.isdirty();
		if(transform_changed) obj->world_aabb();
	
		const glm::mat4& m = obj->transform.model_matrix();
		const vec4<float>& color = obj->color_norm();

		size_t i = 0;
		std::vector<SlotLocation>& slots = this->static_lookup.at(obj);
		for(const Model::SubMesh& sub : obj->model()->meshes()) {
			SlotLocation& loc = slots[i++];
			// Write directly into the flattened buffer
			this->static_instances[loc.batch->instance_index + loc.index] = Renderable::InstanceData {
				m, color, sub.material.metallic, sub.material.roughness
			};
			
			if(transform_changed) touched_batches.insert(loc.batch);
		}
	}

	// If a full rebuild got triggered, skip AABB/visibility patching
	if(!this->static_dirty) {
		for(Batch* batch : touched_batches) {
			batch->aabb = AABB{};
			for(Renderable* owner : batch->owners) batch->aabb.merge(owner->world_aabb());
			batch->visible = this->frustum.test(batch->aabb);
		}
	}

	this->dirty_queue.clear();
}

// -------

void Renderer::begin_frame() noexcept {
	this->pass_index = -1;
	this->frame_index = (this->frame_index + 1) % PersistentMappedBuffer::FRAMES_IN_FLIGHT; // Cap to ring buffer size
	this->static_rebuilt_this_frame = false;

	if(this->static_dirty) {
		this->flatten_persistent();
		this->static_dirty = false;
		this->static_rebuilt_this_frame = true;
	} else if(!this->dirty_queue.empty()) {
		this->patch_dirty();
	}
}

void Renderer::begin_draw(const Camera& camera, const bool cullface) noexcept {
	static bool lastcullface = false;
	if(cullface != lastcullface) {
		lastcullface = cullface;
		if(cullface) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	}

	this->pass_index++;
	this->cached_view = camera.view();
	this->cached_proj = camera.proj();
	this->frustum.update(this->cached_proj * this->cached_view);

	// Find this camera's history
	CameraHistory* hist = nullptr;
	for(CameraHistory& h : this->camera_history) {
		if(h.camera == &camera) {
			hist = &h;
			break;
		}
	}

	if(!hist) {
		this->camera_history.push_back({
			&camera,
			camera.position,
			camera.forward
		});
		this->camera_dirty = true; // first time
	} else {
		this->camera_dirty = glm::distance(hist->position, camera.position) > 1e-6f
			|| glm::distance(hist->forward, camera.forward) > 1e-6f;
		hist->position = camera.position;
		hist->forward = camera.forward;
	}

	this->dynamic_objs.clear();
	this->dynamic_batches.clear();
	this->static_included = false;
}

void Renderer::add(Renderable& obj) noexcept {
	// const size_t index = this->static_objs.size();
	this->static_objs.push_back(&obj);

	obj.is_persistent = true;
	// obj.index = index;
	obj.transform.on_dirty = [this, &obj]() {
		if(!obj.is_dirty_queued) {
			this->dirty_queue.push_back(&obj);
			obj.is_dirty_queued = true;
		}
	};

	this->mark_dirty(); // Forces 'flatten_persistent' next 'begin_frame'
	// return index;
}

void Renderer::draw(Renderable& obj) noexcept {
	if(obj.transform.isdirty() || this->camera_dirty) {
		obj.visible = this->frustum.test(obj.world_aabb());
	}
	if(!obj.visible) return;
	this->dynamic_objs.push_back(&obj);
}

void Renderer::draw_persistent() noexcept {
	this->static_included = true;
	// Re-test every batch if the camera moved or a full rebuild
	if(this->camera_dirty || this->static_rebuilt_this_frame) {
		for(auto& [key, batch] : this->static_batches) {
			batch.visible = this->frustum.test(batch.aabb);
		}
	}
}

void Renderer::flush() noexcept {
	// Build dynamic batches
	this->dynamic_batches.clear();
	for(Renderable* obj : this->dynamic_objs) {
		this->insert_dynamic(*obj);
	}

	// Static instances
	const size_t base = (this->static_included) ? this->static_instances.size() : 0;
	size_t offset = base;
	for(auto& [key, batch] : this->dynamic_batches) {
		batch.instance_index = static_cast<u32>(offset);
		offset += batch.gpu_data.size();
	}
}

} // namespace floyd

