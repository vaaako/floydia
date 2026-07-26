#include "floydia/core/renderer.hpp"
#include "floydia/core/core.hpp"
#include "floydia/geometry/font.hpp"
#include "floydia/rendering/light.hpp"
#include <algorithm>
#include <unordered_set>

#if defined(FLOYD_DEBUG_RENDERER)
#include "floydia/helpers/logger.hpp"
#endif

namespace floyd {

Renderer::Renderer() noexcept :
	ubo_camera(0, sizeof(Camera::CameraData) * 2),
	ssbo_objs(1, sizeof(Renderable::InstanceData) * 128),
	ssbo_lights(2, sizeof(Light::LightData) * 10),
	ssbo_glyphs(3, sizeof(Font::GlyphData) * 128),
	ppipeline(ProgramPipeline())
	{
	
	glGenVertexArrays(1, &this->emptyvao);
	this->ppipeline.bind();

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::update_viewport(const u32 width, const u32 height) noexcept {
	this->wwidth = width;
	this->wheight = height;
	assets().defaults.PROG_VERT_TEXT->set_uniform_vec2f("u_screen_size", { (float)width, (float)height });
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
		Batch& batch = this->resolve_batch(sub, this->static_batches);
		u32 idx = batch.push_static({
			m, color, sub.material.metallic, sub.material.roughness, {}
		}, &obj);
		slots.push_back({ &batch, idx });
	}
}

void Renderer::insert_dynamic(Renderable& obj) noexcept {
	const glm::mat4& m = obj.transform.model_matrix();
	const vec4<float>& color = obj.color_norm();

	for(const Model::SubMesh& sub : obj.model()->meshes()) {
		Batch& batch = this->resolve_batch(sub, this->dynamic_batches);
		batch.push_dynamic({
			m, color, sub.material.metallic, sub.material.roughness, {}
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
				m, color, sub.material.metallic, sub.material.roughness, {}
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

void Renderer::upload_objects() noexcept {
	const size_t static_size = (this->static_included)
		? this->static_instances.size() * sizeof(Renderable::InstanceData) : 0;

	size_t dynamic_size = 0;
	for(auto& [key, batch] : this->dynamic_batches) {
		dynamic_size += batch.gpu_data.size() * sizeof(Renderable::InstanceData);
	}

	const size_t total_size = static_size + dynamic_size;
	if(total_size == 0) return; // Nothing to upload
	if(!this->wrote_objs) { this->ssbo_objs.wait(this->frame_index); this->wrote_objs = true; }

	const size_t needed = this->ssbo_objs_pass_offset + total_size;
	this->ssbo_objs.ensure_capacity(needed);

	const size_t pass_base = this->ssbo_objs.frame_offset(this->frame_index) + this->ssbo_objs_pass_offset;
	// Subtracting converts 'instance_index' into a byte offset relative to 'pass_base', without relying on iteration order
	// ('base' matches the offset 'build_dynamic_batches')
	const size_t base = (this->static_included) ? this->static_instances.size() : 0;
	if(static_size > 0) {
		this->ssbo_objs.update(this->static_instances.data(), static_size, pass_base);
	}

	for(auto& [key, batch] : this->dynamic_batches) {
		if(batch.gpu_data.empty()) continue;
		const size_t batch_bytes = batch.gpu_data.size() * sizeof(Renderable::InstanceData);
		const size_t batch_offset = pass_base + static_size + (batch.instance_index - base) * sizeof(Renderable::InstanceData);
		this->ssbo_objs.update(batch.gpu_data.data(), batch_bytes, batch_offset);
	}

	this->ssbo_objs.flush(pass_base, total_size);
	this->ssbo_objs_pass_offset += total_size;
}

void Renderer::upload_camera() noexcept {
	const Camera::CameraData cam {
		.view = this->cached_view,
		.proj = this->cached_proj,
		.camerapos = vec4<float>(this->campos, 1.0f)
	};

	const size_t needed = (this->pass_index + 1) * sizeof(Camera::CameraData);
	if(needed == 0) return; // Nothing to upload
	if(!this->wrote_camera) { this->ubo_camera.wait(this->frame_index); this->wrote_camera = true; }

	this->ubo_camera.ensure_capacity(needed);
	const size_t offset = this->ubo_camera.frame_offset(this->frame_index)
		+ this->pass_index * sizeof(Camera::CameraData);

	this->ubo_camera.update(&cam, sizeof(Camera::CameraData), offset);
	this->ubo_camera.flush(offset, sizeof(Camera::CameraData));
}

void Renderer::upload_lights() noexcept {
	if(this->wrote_lights) return; // Already sent to SSBO

	const size_t header_size  = sizeof(Light::LightBuffer); // Readability
	const size_t static_size  = (this->static_included) ? this->static_lights.size() * sizeof(Light::LightData) : 0;
	const size_t dynamic_size = this->dynamic_lights.size() * sizeof(Light::LightData);
	const size_t total_size   = header_size + static_size + dynamic_size;
	if(static_size == 0 && dynamic_size == 0) return; // Nothing to upload
	this->ssbo_lights.wait(this->frame_index); this->wrote_lights = true;

	this->ssbo_lights.ensure_capacity(total_size);
	const size_t offset = this->ssbo_lights.frame_offset(this->frame_index);

	// Upload header first
	Light::LightBuffer header;
	header.count = ((this->static_included) ? this->static_lights.size() : 0) + this->dynamic_lights.size();
	this->ssbo_lights.update(&header, header_size, offset);

	// Upload after header
	if(static_size > 0) {
		this->ssbo_lights.update(this->static_lights.data(), static_size, offset + header_size);
	}

	// Upload after header and static lights
	if(dynamic_size > 0) {
		this->ssbo_lights.update(this->dynamic_lights.data(), dynamic_size, offset + header_size + static_size);
	}

	this->ssbo_lights.flush(offset, total_size);
}

void Renderer::upload_text() noexcept {
	if(this->glyphs.empty()) return;

	const size_t size = this->glyphs.size() * sizeof(Font::GlyphData);
	if(!this->wrote_glyphs) { this->ssbo_glyphs.wait(this->frame_index); this->wrote_glyphs = true; }

	this->ssbo_glyphs.ensure_capacity(size);
	const size_t offset = this->ssbo_glyphs.frame_offset(this->frame_index);

	this->ssbo_glyphs.update(this->glyphs.data(), size, offset);
	this->ssbo_glyphs.flush(offset, size);
}

void Renderer::render_batches(const BatchTable& table) noexcept {
	this->render_scratch.clear();
	for(auto& [key, batch] : table) {
		if(batch.instance_count == 0 || !batch.visible) continue;
		this->render_scratch.push_back(&batch);
	}
	if(this->render_scratch.empty()) return;

	std::sort(this->render_scratch.begin(), this->render_scratch.end(), [](const Batch* a, const Batch* b) {
		if(a->material->vertex.get() != b->material->vertex.get()) return (a->material->vertex.get() < b->material->vertex.get());
		if(a->material->fragment.get() != b->material->fragment.get()) return (a->material->fragment.get() < b->material->fragment.get());
		if(a->material->texture() != b->material->texture()) return (a->material->texture() < b->material->texture());
		return a->mesh < b->mesh;
	});

	ShaderProgram* prev_vertex = nullptr;
	ShaderProgram* prev_fragment = nullptr;
	const Material* prev_material = nullptr;
	Mesh* prev_mesh = nullptr;

	for(const Batch* batch : this->render_scratch) {
		if(batch->mesh != prev_mesh) {
			prev_mesh = batch->mesh;
			glBindVertexArray(prev_mesh->vaoid());
		}

		if(batch->material != prev_material) {
			if(batch->material->vertex.get() != prev_vertex) {
				this->ppipeline.attach(batch->material->vertex, Shader::Vertex);
				prev_vertex = batch->material->vertex.get();
			}

			if(batch->material->fragment.get() != prev_fragment) {
				this->ppipeline.attach(batch->material->fragment, Shader::Fragment);
				prev_fragment = batch->material->fragment.get();
			}

			batch->material->bind();
			prev_material = batch->material;
		}

		glDrawElementsInstancedBaseInstance(
			GL_TRIANGLES,
			batch->mesh->index_count,
			batch->mesh->index_type,
			(void*)0,
			batch->instance_count,
			batch->instance_index // gl_BaseInstance
		);
	}
}

void Renderer::render_text() noexcept {
	if(this->text_batches.empty()) return;

	this->ppipeline.attach(assets().defaults.PROG_VERT_TEXT, Shader::Vertex);
	this->ppipeline.attach(assets().defaults.PROG_FRAG_2D, Shader::Fragment);

	glBindVertexArray(this->emptyvao);
	glDepthMask(GL_FALSE); // Text draws over everything, doesn't need depth write

	for(auto& [font, batch] : this->text_batches) {
		if(batch.glyph_count == 0) continue;
		font->atlas()->bind(0);

		glDrawArraysInstancedBaseInstance(
			GL_TRIANGLES, 0, 6,
			batch.glyph_count, batch.glyph_start
		);
	}

	glDepthMask(GL_TRUE);
	glBindVertexArray(0);
}


// -------


void Renderer::begin_frame(const vec4<float>& clear_color) noexcept {
	glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	this->pass_index = -1;
	this->ssbo_objs_pass_offset = 0;
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

void Renderer::end_frame() noexcept {
	if(this->wrote_camera) this->ubo_camera.lock(this->frame_index);
	if(this->wrote_objs)   this->ssbo_objs.lock(this->frame_index);
	if(this->wrote_lights) this->ssbo_lights.lock(this->frame_index);
	if(this->wrote_glyphs) this->ssbo_glyphs.lock(this->frame_index);
	this->wrote_camera = this->wrote_objs = this->wrote_lights = this->wrote_glyphs = false;
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
	this->campos = camera.position;

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

	this->dynamic_batches.clear();
	this->dynamic_objs.clear();
	this->glyphs.clear();
	this->text_batches.clear();
	this->static_included = false;
}

void Renderer::add(Renderable& obj) noexcept {
	// NOTE: RTTI (dynamic_cast) is acceptable here, `add()` is rarely called
	if(dynamic_cast<Font*>(&obj) != nullptr) {
		TRACELOG(logger::Error, "Text objects cannot be added as persistent. Please use 'draw_text()'");
		return;
	} else if(dynamic_cast<Sprite*>(&obj) != nullptr) {
		TRACELOG(logger::Error, "2D objects cannot be added as persistent. Please use 'draw()' directly");
		return;
	}

	this->static_objs.push_back(&obj);
	obj.is_persistent = true;
	obj.transform.on_dirty = [this, &obj]() {
		if(!obj.is_dirty_queued) {
			this->dirty_queue.push_back(&obj);
			obj.is_dirty_queued = true;
		}
	};
	this->mark_dirty(); // Forces 'flatten_persistent' next 'begin_frame'
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

void Renderer::draw_text(const std::string& text, const vec2<float>& pos, const std::shared_ptr<Font>& font,
	const float scale, const vec4<float>& color) noexcept {

	if(text.empty() || !font) return;

	auto [it, inserted] = this->text_batches.try_emplace(font.get(),
		TextBatch { font.get(), static_cast<u32>(this->glyphs.size()), 0 });
	TextBatch& batch = it->second;

	const float ascent = font->ascent();
	const u8* p = reinterpret_cast<const u8*>(text.c_str());
	float pen_x = pos.x;
	float pen_y = pos.y;

	while(*p) {
		const u32 codepoint = font->utf8_next(p);
		if(codepoint == '\n') {
			pen_x = pos.x;
			pen_y += font->line_height() * scale;
			continue;
		}

		const Font::Glyph g = font->glyph(codepoint, scale);
		if(g.width > 0.0f && g.height > 0.0f) {
			Font::GlyphData gd;
			gd.pos   = { pen_x + g.offset_x, pen_y + ascent + g.offset_y };
			gd.size  = { g.width, g.height };
			gd.uv0   = g.uv0;
			gd.uv1   = g.uv1;
			gd.color = color;
			this->glyphs.push_back(gd);
			batch.glyph_count++;
		}
		pen_x += g.advance;
	}
}

void Renderer::remove(Renderable& obj) noexcept {
	if(!obj.is_persistent) return;
	this->static_objs.erase(
		std::remove(this->static_objs.begin(), this->static_objs.end(), &obj),
		this->static_objs.end()
	);
	this->static_lookup.erase(&obj);

	obj.is_persistent = false;
	obj.is_dirty_queued = false;
	obj.needs_rebatch = false;

	this->mark_dirty();
}

void Renderer::flush() noexcept {
	// -- Build Instances

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
	if(this->dynamic_batches.empty() && this->static_batches.empty() && this->glyphs.empty()) return;

	// -- Upload objects to SSBO
	this->upload_camera();
	this->upload_objects();
	this->upload_lights();
	this->upload_text();

	// -- Render batches
	if(this->static_included) this->render_batches(this->static_batches);
	this->render_batches(this->dynamic_batches);
	this->render_text();
}

} // namespace floyd

