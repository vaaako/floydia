#pragma once

#include "floydia/geometry/text.hpp"
#include <floydia/rendering/light.hpp>
#include <floydia/gpu/programpipeline.hpp>
#include <floydia/camera/frustum.hpp>
#include <floydia/types.hpp>
#include <floydia/camera/camera.hpp>
#include <floydia/rendering/renderable.hpp>
#include <floydia/geometry/cube.hpp>
#include <floydia/rendering/mesh.hpp>
#include <floydia/material/material.hpp>
#include <floydia/gpu/uniformbuffer.hpp>
#include <floydia/gpu/ssbo.hpp>
#include <floydia/helpers/hash.hpp>

#include <vector>

namespace floyd {

class Window;

class Renderer final {
	public:
		Renderer() noexcept;
		~Renderer() = default;

		// Update stored width and height values
		void update_viewport(const uint32 width, const uint32 height) noexcept;
		// Changes the clear color
		void set_clear_color(const vec4<uint8>& color) noexcept;
		// Clear the screen
		void clear() const noexcept;

		// Advances 'frameindex' and waits for the GPU to finish reading
		// the current frame's buffer slots before the CPU writes new data
		void begin_frame() noexcept;
		// Singals GPU fences to mark this frame's buffer slots as in-flight,
		// preventing the CPU from overwriting them before the GPU is done
		void end_frame() noexcept;
		// NOTE: both avoid 'wait', 'lock' and 'frameindex' running multiple times per frame

		// Advances the frame index, syncs GPU fences, updates camera UBO,
		// updates the frustum, and rebuilds persistent batches if dirty
		void begin_draw(const Window& window, const Camera& camera) noexcept;

		// Includes persistent objects in the current pass.
		// Must be called after 'begin_draw()' and before 'flush()'.
		// Persistent objects are only rebuilt when dirty (i.e. when 'add()' is called).
		// Without this call, persistent objects are excluded from the current pass
		void draw_persistent() noexcept;
		// Submit a dynamic object for this frame. Frustum culled
		void draw(Renderable& obj) noexcept;
		// Submit a persistent object. Batched once and reused every frame.
		// Skips per-frame frustum culling
		size_t add(const Renderable& obj) noexcept;
		// Submit a dynamic object for this frame
		void draw(const Light& light) noexcept;
		// Submit a persistent light object
		size_t add(const Light& light) noexcept;

		// Submit a text object for this frame
		void draw_text(const std::string& text, const vec2<float>& pos, const std::shared_ptr<Text>& font,
			const float scale = 1.0f, const vec4<float>& color = vec4<float>(1.0f)) noexcept;

		// Upload instance data to SSBo and issue draw calls
		void flush() noexcept;

	private:
		struct DrawBatch {
			Mesh* mesh;
			// MaterialInstance holds textures + points to cached Material (Shaders)
			// BatchKey uses GPU IDs so different instances with same state share a batch
			MaterialInstance* matinst;
			std::vector<Renderable::InstanceData> instances;
			// Start index in SSBO
			uint32 instance_index; // start offset into the instance buffer
			// Number of instances for this mesh
			uint32 instance_count;
		};

		struct TextBatch {
			Text* font;
			// First gl_InstanceID on SSBO
			u32 glyph_start;
			u32 glyph_count;
		};

		struct BatchKey {
			Mesh* mesh;
			GLuint vert_id;
			GLuint frag_id;
			GLuint albedo_id;
			float metallic;
			float roughness;
			bool operator==(const BatchKey& other) const noexcept {
				return mesh   == other.mesh     &&
					vert_id   == other.vert_id  &&
					frag_id   == other.frag_id  &&
					albedo_id == other.albedo_id &&
					metallic == other.metallic &&
					roughness == other.roughness;
			}
		};

		struct BatchKeyHash {
			std::size_t operator()(const BatchKey& k) const noexcept {
				size_t seed = 0;
				hash::combine(seed, std::hash<Mesh*>()(k.mesh));
				hash::combine(seed, std::hash<GLuint>()(k.vert_id));
				hash::combine(seed, std::hash<GLuint>()(k.frag_id));
				hash::combine(seed, std::hash<GLuint>()(k.albedo_id));
				hash::combine(seed, std::hash<float>()(k.metallic));
				hash::combine(seed, std::hash<float>()(k.roughness));
				return seed;
			}
		};

	private:
		// Dynamic batches, rebuilt every frame
		std::unordered_map<BatchKey, DrawBatch, BatchKeyHash> dynamic_batches;
		// Pre-built batches from persistent objects, rebuilt only when dirty
		std::unordered_map<BatchKey, DrawBatch, BatchKeyHash> persistent_batches;
		// All instance data, uploaded to the SSBO each frame
		std::vector<Renderable::InstanceData> instances;
		std::vector<Renderable::InstanceData> persistent_instances;
		// Persistent object pointers, stored by pointer to avoid copies
		std::vector<const Renderable*> persistent_objs;

		// All instance data, uploaded to the SSBO each frame
		std::vector<Light::LightData> lights;
		// Persistent light pointers, stored by pointer to avoid copies
		std::vector<const Light*> persistent_lights;

		std::vector<TextBatch> text_batches;
		std::vector<Text::GlyphData> glyphs;


		glm::mat4 cached_view;
		glm::mat4 cached_proj;
		glm::mat4 last_vp;
		vec3<float> camerapos;

		float clear_color[4] = { 0.1f, 0.1f, 0.1f, 0.1f };

		Frustum frustum;
		UniformBuffer ubo_camera;
		ShaderStorageBuffer ssbo_objs;
		ShaderStorageBuffer ssbo_lights;
		ShaderStorageBuffer ssbo_glyphs;
		ProgramPipeline ppipeline;

		u32 total_text_instances = 0; // Only used to resize 'ssbo_glyphs'
		GLuint text_vao; // empty vao to satisfy core profile
		
		// Cache if necessary
		uint32 win_width;
		uint32 win_height;
		uint32 frameindex = 0;
		// Counts down from FRAMES_IN_FLIGHT to ensure persistent data
		// is uploaded to every buffer slot after a change
		uint32 persistent_ssbo_objs_dirty;
		uint32 persistent_ssbo_light_dirty;
		int pass_index = -1; // Tracks the current pass within a frame. Incremented on each begin_draw call
							 // Used to offset the camera UBO so eac pass has its own camera slot without overwriting others
		bool camera_dirty = true; // Check if camera moved
		bool persistent_dirty = true; // Track if persistent batch cache is dirty
		bool persistent_included = false; // Set by draw_persistent() to indicate persistant objects should be draw this pass
										  // Without this flag, draw_map(persistent_batches) would run every flush() with stale instance_index
										  // values, corrupting the SSBO reads for dynamic objects

	private:
		bool camera_moved(const glm::mat4& vp) noexcept;
		void add_batch(const Renderable& obj, std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& target) noexcept;
		void draw_map(const std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& batchmap) const noexcept;
		void draw_text_batches() noexcept;
};

} // namespace floyd
