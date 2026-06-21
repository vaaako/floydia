#pragma once

#include "floydia/camera/perspectivecamera.hpp"
#include "floydia/geometry/text.hpp"
#include "floydia/rendering/light.hpp"
#include "floydia/gpu/programpipeline.hpp"
#include "floydia/camera/frustum.hpp"
#include "floydia/types.hpp"
#include "floydia/camera/camera.hpp"
#include "floydia/rendering/renderable.hpp"
#include "floydia/rendering/mesh.hpp"
#include "floydia/gpu/uniformbuffer.hpp"
#include "floydia/gpu/ssbo.hpp"
#include "floydia/helpers/hash.hpp"

#include <vector>

namespace floyd {

class Window;

class Renderer final {
	friend struct SceneBuilder;

	public:
		Renderer() noexcept;
		~Renderer() = default;

		// Casts a ray from the mouse position and returns the closest visible object hit.
		// Returns nullptr if nothing was hit.
		// Call AFTER 'begin_frame()', otherwise dynamic objects won't be included
		Renderable* pick(const PerspectiveCamera& camera, const vec2<u32>& mouse_pos, const vec2<u32>& win_size) const noexcept;
		// Update stored width and height values
		void update_viewport(const u32 width, const u32 height) noexcept;
		// Changes the clear color
		void set_clear_color(const vec4<u8>& color) noexcept;
		// Mark persistent objects as dirty
		void mark_dirty() noexcept;
		// Debug draw an AABB.
		// Should not be used on release since it is a debug method
		void draw_aabb(const AABB& aabb, const vec4<float>& color) noexcept;

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
		void begin_draw(const Camera& camera) noexcept;

		// Includes persistent objects in the current pass.
		// Must be called after 'begin_draw()' and before 'flush()'.
		// Persistent objects are only rebuilt when dirty (i.e. when 'add()' is called).
		// Without this call, persistent objects are excluded from the current pass
		void draw_persistent() noexcept;

		// Submit a dynamic object for this frame.
		// Use this for temporary objects, or objects that changes often
		void draw(Renderable& obj) noexcept;
		// Submit a persistent object.
		// Use this for objects that rarely changes properties
		size_t add(Renderable& obj) noexcept;
		// Removes a persistent objcet from the renderer
		void remove(Renderable& obj) noexcept;

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
			// world AABB for batch
			AABB aabb;
			// To send to SSBO
			std::vector<Renderable::InstanceData> instances;
			// All objects in this batch
			std::vector<Renderable*> objects;
			// Start index in SSBO
			u32 instance_index; // start offset into the instance buffer
			// Number of instances for this mesh
			u32 instance_count;
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
			// Metallic and Roughness removed.
			// Hashing floats is fragile: values that differ only by floating-point
			// noise produce different hashes and split batches that should be merged.
			// Per-instance material properties belong in InstanceData on the SSBO, not in the batch key
			bool operator==(const BatchKey& other) const noexcept {
				return mesh   == other.mesh     &&
					vert_id   == other.vert_id  &&
					frag_id   == other.frag_id  &&
					albedo_id == other.albedo_id;
			}
		};

		struct BatchKeyHash {
			std::size_t operator()(const BatchKey& k) const noexcept {
				size_t seed = 0;
				hash::combine(seed, std::hash<Mesh*>()(k.mesh));
				hash::combine(seed, std::hash<GLuint>()(k.vert_id));
				hash::combine(seed, std::hash<GLuint>()(k.frag_id));
				hash::combine(seed, std::hash<GLuint>()(k.albedo_id));
				return seed;
			}
		};

	private:
		// Dynamic batches, rebuilt every frame
		std::unordered_map<BatchKey, DrawBatch, BatchKeyHash> dynamic_batches;
		// Pre-built batches from persistent objects, rebuilt only when dirty. Used to draw batches
		std::unordered_map<BatchKey, DrawBatch, BatchKeyHash> persistent_batches;
		// All instance data, uploaded to the SSBO each frame
		std::vector<Renderable::InstanceData> instances;
		std::vector<Renderable::InstanceData> persistent_instances;

		// Persistent object pointers, stored by pointer to avoid copies
		std::vector<Renderable*> persistent_objs;
		// Persistent objs dirty this frame
		std::vector<size_t> dirty_queue;
		// Dynamic objects. For ray picking only. Cleared on 'begin_frame'
	
	#if !defined(FLOYD_RELEASE)
		std::vector<Renderable*> pickables;
		ShaderProgram aabb_program;
	#endif

		// All light instances to upload to SSBO
		// dynamic lights and persistent_lights
		std::vector<Light::LightData> lights;
		// Persistent light pointers, stored by pointer to avoid copies
		std::vector<Light::LightData> persistent_lights;

		std::unordered_map<Text*, TextBatch> text_batches;
		std::vector<Text::GlyphData> glyphs;


		glm::mat4 cached_view;
		glm::mat4 cached_proj;
		vec3<float> campos;
		vec3<float> camforward; // Cheap check when camera moved

		float clear_color[4] = { 0.1f, 0.1f, 0.1f, 0.1f };

		Frustum frustum;
		UniformBuffer ubo_camera;
		ShaderStorageBuffer ssbo_objs;
		ShaderStorageBuffer ssbo_lights;
		ShaderStorageBuffer ssbo_glyphs;
		ProgramPipeline ppipeline;

		u32 total_text_instances = 0; // Only used to resize 'ssbo_glyphs'
		GLuint empty_vao; // empty vao to satisfy core profile
		
		// Cache if necessary
		u32 win_width;
		u32 win_height;
		u32 frameindex = 0;
		// Counts down from FRAMES_IN_FLIGHT to ensure persistent data
		// is uploaded to every buffer slot after a change
		u32 persistent_ssbo_objs_dirty;
		u32 persistent_ssbo_light_dirty;
		int pass_index = -1; // Tracks the current pass within a frame. Incremented on each begin_draw call
							 // Used to offset the camera UBO so eac pass has its own camera slot without overwriting others
		size_t ssbo_objs_pass_offset = 0; // Tracks current pass within frame. Other SSBOs do not appear on other passes, no need
		bool camera_dirty = true; // Check if camera moved
		bool persistent_dirty = true; // Track if persistent batch cache is dirty
		bool persistent_included = false; // Set by draw_persistent() to indicate persistant objects should be draw this pass
										  // Without this flag, draw_map(persistent_batches) would run every flush() with stale instance_index
										  // values, corrupting the SSBO reads for dynamic objects

	private:
		bool camera_moved(const vec3<float>& campos, const vec3<float>& forward) noexcept;
		void add_batch(Renderable& obj, std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& target, u32* out_slot = nullptr) noexcept;
		void draw_map(const std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& batchmap, const bool cull) noexcept;
		void draw_text_batches() noexcept;
};

} // namespace floyd
