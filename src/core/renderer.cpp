#include "floydia/physics/ray.hpp"
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
	ubo_camera(0, sizeof(Camera::CameraData)),
	ssbo_objs(1, sizeof(Renderable::InstanceData) * 128),
	ssbo_lights(2, sizeof(Light::LightData) * 10),
	ssbo_glyphs(3, sizeof(Text::GlyphData) * 256),
	ppipeline(ProgramPipeline())
	{

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// glEnable(GL_CULL_FACE);

	glGenVertexArrays(1, &this->text_vao);
	this->ppipeline.bind();
}

Renderable* Renderer::pick(const PerspectiveCamera& camera, const vec2<u32>& mouse_pos, const vec2<u32>& win_size) const noexcept {
	Ray ray = Ray::screen_to_ray(camera, mouse_pos, win_size);
	Renderable* obj = nullptr;
	float obj_t = std::numeric_limits<float>::max();

	auto test = [&](Renderable* r) {
		if(r == nullptr || !r->visible || r->world_aabb().is_2d) return;
		const float t = ray.test_aabb(r->world_aabb());
		// 100.0f here is the max distance
		if(t >= 0.0f && t < 100.0f && t < obj_t) {
			obj_t = t;
			obj = r;
		}
	};

	for(Renderable* r : this->persistent_objs) {
		test(r);
	}

	for(Renderable* r : this->pickables) {
		test(r);
	}

	return obj;
}

void Renderer::update_viewport(const u32 width, const u32 height) noexcept {
	this->win_width = width;
	this->win_height = height;
	assets().defaults.PROG_VERT_TEXT->set_uniform_vec2f("u_screen_size", { this->win_width, this->win_height });
}

void Renderer::set_clear_color(const vec4<u8>& color) noexcept {
	this->clear_color[0] = color.x / 255.0f;
	this->clear_color[1] = color.y / 255.0f;
	this->clear_color[2] = color.z / 255.0f;
	this->clear_color[3] = color.w / 255.0f;
}

void Renderer::mark_dirty() noexcept {
	this->persistent_dirty = true;
	this->persistent_ssbo_objs_dirty = PersistentMappedBuffer::FRAMES_IN_FLIGHT; // upload to all frame slots
}

void Renderer::clear() const noexcept {
	glClearColor(this->clear_color[0], this->clear_color[1], this->clear_color[2], this->clear_color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::begin_frame() noexcept {
	this->pass_index = -1; // Reset
	// Advance frame
	this->frameindex = (this->frameindex + 1) % PersistentMappedBuffer::FRAMES_IN_FLIGHT; // Cap to ring buffer size
	// Block CPU until GPU is done reading this frame's buffer slots.
	this->ubo_camera.wait(this->frameindex); // Correct but overkill
	this->ssbo_objs.wait(this->frameindex);
	this->ssbo_lights.wait(this->frameindex);
	this->ssbo_glyphs.wait(this->frameindex);

	this->pickables.clear();
}

void Renderer::end_frame() noexcept {
	// Signal GPU fence, marks this frame's buffer slots as in-flight
	this->ubo_camera.lock(this->frameindex);
	this->ssbo_objs.lock(this->frameindex);
	this->ssbo_lights.lock(this->frameindex);
	this->ssbo_glyphs.lock(this->frameindex);
}

void Renderer::begin_draw(const Camera& camera) noexcept {
	this->pass_index++;
	// Clear previous frame
	this->dynamic_batches.clear();
	this->instances.clear();
	this->lights.clear();
	this->text_batches.clear();
	this->glyphs.clear();
	this->total_text_instances = 0;
	this->persistent_included = false;

	// Update Camera and Camera Uniform Buffer
	this->cached_view = camera.view();
	this->cached_proj = camera.projection();

	// Update frustum once per frame
	const glm::mat4 vp = this->cached_proj * this->cached_view;
	this->frustum.update(vp);
	this->camera_dirty = this->camera_moved(camera.position, camera.forward); // this->campos set here

	// Rebuild persistent batch cache only when dirty
	if(this->persistent_dirty) {
		this->persistent_batches.clear();
		this->persistent_instances.clear();
		for(Renderable* obj : this->persistent_objs) {
			if(!obj) continue;
			this->add_batch(*obj, this->persistent_batches);
		}

		// Build instances to send to SSBO
		for(auto& [_, batch] : this->persistent_batches) {
			batch.instance_index = this->persistent_instances.size();
			// Bulk copy. Insert all of batch's instances into 'persistent_instances'
			this->persistent_instances.insert(
				this->persistent_instances.end(),
				batch.instances.begin(),
				batch.instances.end()
			);
		}

		this->persistent_dirty = false;
	}
}

void Renderer::draw_persistent() noexcept {
	this->persistent_included = true;
	// Rebuild persistent lights
	for(const Light* light : this->persistent_lights) {
		this->lights.push_back(light->to_gpu_data());
	}
}

void Renderer::draw(Renderable& obj) noexcept {
	// Only cull if obj changed or camera moved
	if(obj.transform.isdirty()) {
		obj.visible = this->frustum.test(obj.world_aabb());
	} else if(this->camera_dirty) {
		obj.visible = this->frustum.test(obj.world_aabb());
	}
	if(!obj.visible) return;
	this->add_batch(obj, this->dynamic_batches);
	this->pickables.push_back(&obj); // For Ray picking
}

size_t Renderer::add(Renderable& obj) noexcept {
	this->persistent_objs.push_back(&obj);
	this->persistent_dirty = true;
	this->persistent_ssbo_objs_dirty = PersistentMappedBuffer::FRAMES_IN_FLIGHT; // upload to all frame slots
	return this->persistent_objs.size() - 1;
}

void Renderer::draw(const Light& light) noexcept {
	this->lights.push_back(light.to_gpu_data());
}

size_t Renderer::add(const Light& light) noexcept {
	this->persistent_lights.push_back(&light);
	this->persistent_ssbo_light_dirty = PersistentMappedBuffer::FRAMES_IN_FLIGHT; // upload to all frame slots
	return this->persistent_lights.size() - 1;
}

void Renderer::draw_text(const std::string& text, const vec2<float>& pos, const std::shared_ptr<Text>& font, const float scale, const vec4<float>& color) noexcept {
	if(text.empty() || !font) return;

	auto [it, inserted] = this->text_batches.try_emplace(font.get(),
		TextBatch {
			font.get(),
			this->total_text_instances,
			0,
		}
	);
	TextBatch* batch = &it->second;

	const float ascent = font->ascent();
	const u8* p = reinterpret_cast<const u8*>(text.c_str());
	float pen_x = pos.x;
	float pen_y = pos.y;

	while(*p) {
		const u32 codepoint = font->utf8_next(p);
		if(codepoint == '\n') {
			pen_x  = pos.x;
			pen_y += font->line_height() * scale;
			continue;
		}

		const Text::Glyph g = font->glyph(codepoint, scale);
		if(g.width > 0.0f && g.height > 0.0f) {
			Text::GlyphData gd;
			// gd.pos   = { pen_x + g.offset_x, pen_y + g.offset_y };
			gd.pos = { pen_x + g.offset_x, pen_y + ascent + g.offset_y };
			gd.size  = { g.width, g.height };
			gd.uv0   = g.uv0;
			gd.uv1   = g.uv1;
			gd.color = color;

			this->glyphs.push_back(gd);
			++batch->glyph_count;
		}
		pen_x += g.advance;
	}
	this->total_text_instances = this->glyphs.size();
}

void Renderer::flush() noexcept {
	// Skip if no object
	if(!this->dynamic_batches.empty() || !this->persistent_batches.empty()) {
		// Update camera here so 'lights' is updated
		const Camera::CameraData cam = { this->cached_view, this->cached_proj, glm::vec4(this->campos, 1.0f) };
	
		const size_t offset = this->ubo_camera.frame_offset(frameindex) 
			+ this->pass_index * sizeof(Camera::CameraData);
		const size_t size = (this->pass_index + 1) * sizeof(Camera::CameraData);
		if(size > this->ubo_camera.perframesize()) this->ubo_camera.resize(size);

		this->ubo_camera.update(&cam, sizeof(Camera::CameraData), offset);
		this->ubo_camera.flush(offset, sizeof(Camera::CameraData)); // Bind for shader use

		// Put on final instances
		if(persistent_included) this->instances.insert(this->instances.end(), this->persistent_instances.begin(), this->persistent_instances.end());
		// Update dynamic batches
		for(auto& [_, batch] : this->dynamic_batches) {
			batch.instance_index = this->instances.size();
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
	}

	// Update SSBO when there are dynamic objects or persistent slots need filling
	if(!this->instances.empty() || this->persistent_ssbo_objs_dirty > 0) {
		// Resize if necessary. Grow with margin
		const size_t size = this->instances.size() * sizeof(Renderable::InstanceData);
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
			header.count = static_cast<u32>(this->lights.size());
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
	if(this->persistent_included) this->draw_map(this->persistent_batches);
	this->draw_map(this->dynamic_batches);
	if(!this->text_batches.empty() || !this->glyphs.empty()) this->draw_text_batches();
}


void Renderer::add_batch(Renderable& obj, std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& target) noexcept {
	// NOTE: Culling for both types of objects may cause problems.
	// If Persistent Object was added not visible, it is likely to never be visible
	// (excepts a new object is added when it is visible)
	const glm::mat4& mmatrix = obj.final_matrix(this->cached_view); // this updates model matrix
	Renderable::InstanceData data = { mmatrix, obj.color_norm() };

	// Create draw commands with instance index
	for(const Model::SubMesh& sub : obj.model()->meshes()) {
		// Cull sub meshes
		// if(!this->frustum.test(sub.mesh->aabb, mmatrix)) continue;

		// Try to find existing batch
		BatchKey key = {
			sub.mesh.get(),
			sub.material->base->vertex->id(),
			sub.material->base->fragment->id(),
			(sub.material->albedo) ? sub.material->albedo->id() : 0,
			sub.material->metallic,
			sub.material->roughness
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
		}
		batch.instance_count++;
		batch.instances.push_back(data);
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

void Renderer::draw_map(const std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& batchmap) const noexcept {
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

void Renderer::draw_text_batches() noexcept {
	// Upload all glyphs at once
	const size_t size = this->glyphs.size() * sizeof(Text::GlyphData);
	if(size > this->ssbo_glyphs.perframesize()) this->ssbo_glyphs.resize(size);

	const size_t base_offset = this->ssbo_glyphs.frame_offset(this->frameindex);
	this->ssbo_glyphs.update(this->glyphs.data(), size, base_offset);
	this->ssbo_glyphs.flush(base_offset, size);

	// Switch pipeline
	this->ppipeline.attach(assets().defaults.PROG_VERT_TEXT, Shader::Vertex);
	this->ppipeline.attach(assets().defaults.PROG_FRAG_2D, Shader::Fragment);

	glBindVertexArray(this->text_vao);
	glDepthMask(GL_FALSE);

	for(const auto& [_, batch] : this->text_batches) {
		if(batch.glyph_count == 0) continue;
		batch.font->atlas()->bind(0);

		glDrawArraysInstancedBaseInstance(
			GL_TRIANGLES,
			0,
			6,
			batch.glyph_count,
			batch.glyph_start
		);
	}

	glDepthMask(GL_TRUE);
	glBindVertexArray(0);
}

bool Renderer::camera_moved(const vec3<float>& campos, const vec3<float>& forward) noexcept {
	if(campos != this->campos || forward != this->camforward) {
		this->campos = campos;
		this->camforward = forward;
		return true;
	}
	return false;
}

} // namespace floyd


