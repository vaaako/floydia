#pragma once

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

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

namespace floyd {

class Renderer final {
	public:
		Renderer() noexcept;
		~Renderer() = default;

		// Clear the screen
		void clear() const noexcept;
		// Changes the clear color
		void set_clear_color(const vec4<uint8>& color) noexcept;
		// Advances the frame index, syncs GPU fences, updates camera UBO,
		// updates the frustum, and rebuilds persistent batches if dirty
		void begin_draw(const Camera& camera) noexcept;

		// Submit a dynamic object for this frame. Frustum culled
		void push(Renderable& obj) noexcept;
		// Submit a persistent object. Batched once and reused every frame.
		// Skips per-frame frustum culling
		size_t add(const Renderable& obj) noexcept;

		// Submit a dynamic object for this frame
		void push(const Light& light) noexcept;
		// Submit a persistent light object
		size_t add(const Light& light) noexcept;
		// Debug a cube on a light source position
		Cube show_light(const Light& light) noexcept;

		// Upload instance data to SSBo and issue draw calls
		void flush() noexcept;

	private:
		struct InstanceData; // forward declared to keep alignment visible at top

		struct DrawBatch {
			Mesh* mesh;
			// MaterialInstance holds textures + points to cached Material (Shaders)
			// BatchKey uses GPU IDs so different instances with same state share a batch
			MaterialInstance* matinst;
			std::vector<InstanceData> instances;
			// Start index in SSBO
			uint32 instance_index; // start offset into the instance buffer
			// Number of instances for this mesh
			uint32 instance_count;
		};

		struct alignas(16) CameraData {
			glm::mat4 view;
			glm::mat4 proj;
			glm::vec4 camerapos; // on std140 vec3 and vec4 has the same size (16 bytes), but the behaviour may be unexpected (some drivers reads vec3 as 12 bytes), vec4 is safer
		};

		struct alignas(16) InstanceData {
			glm::mat4 model;
			glm::vec4 color;
		};

		struct BatchKey {
			Mesh* mesh;
			GLuint vert_id;
			GLuint frag_id;
			GLuint albedo_id;
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
		// Pre-built batches from persistent objects, rebuilt only when dirty
		std::unordered_map<BatchKey, DrawBatch, BatchKeyHash> static_batches;
		// All instance data, uploaded to the SSBO each frame
		std::vector<InstanceData> instances;
		// Persistent object pointers, stored by pointer to avoid copies
		std::vector<const Renderable*> persistent_objs;

		// All instance data, uploaded to the SSBO each frame
		std::vector<Light::LightData> lights;
		// Persistent light pointers, stored by pointer to avoid copies
		std::vector<const Light*> persistent_lights;

		glm::mat4 cached_view;
		glm::mat4 cached_proj;
		vec3<float> camerapos;
		glm::mat4 last_vp;
		Frustum frustum;
		UniformBuffer ubo_camera;
		ShaderStorageBuffer ssbo_objs;
		ShaderStorageBuffer ssbo_lights;
		ProgramPipeline ppipeline;
		float clear_color[4] = { 0.1f, 0.1f, 0.1f, 0.1f };

		size_t total_instances = 0; // Only used to resize 'instances'

		uint32 frameindex = 0;
		// Counts down from FRAMES_IN_FLIGHT to ensure persistent data
		// is uploaded to every buffer slot after a change
		uint32 persistent_ssbo_objs_dirty;
		uint32 persistent_ssbo_light_dirty;
		bool camera_dirty = true; // Check if camera moved
		bool persistent_dirty = true; // Track if persistent batch cache is dirty

	private:
		inline bool camera_moved(const glm::mat4& vp) noexcept {
			if(vp != this->last_vp) {
				this->last_vp = vp;
				return true;
			}
			return false;
		}
		void add_batch(const Renderable& obj, std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& target) noexcept;
		void draw_map(const std::unordered_map<BatchKey, DrawBatch, BatchKeyHash>& batchmap) const noexcept;
};

} // namespace floyd
