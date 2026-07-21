#pragma once

#include "floydia/camera/camera.hpp"
#include "floydia/camera/frustum.hpp"
#include "floydia/gpu/programpipeline.hpp"
#include "floydia/gpu/ssbo.hpp"
#include "floydia/gpu/uniformbuffer.hpp"
#include "floydia/helpers/hash.hpp"
#include "floydia/rendering/mesh.hpp"
#include "floydia/rendering/renderable.hpp"

namespace floyd {

class Renderer final {
	public:
		Renderer() noexcept;
		~Renderer() = default;

		// Advances the frame index and rebuilds or patches static batches.
		// Call once per frame, before `begin_draw`
		void begin_frame() noexcept;
		// GPU fence signaling for this frame's buffer slots.
		// Call once per frame
		void end_frame() noexcept;
		// Starts a new camera pass.
		// Call once, after `begin_frame` and before `flush`
		void begin_draw(const Camera& camera, const bool cullface = false) noexcept;

		// Includes static objects in the current pass
		void draw_persistent() noexcept;
		// Registers a static object
		void draw(Renderable& obj) noexcept;
		// Unregisters a static object
		void remove(Renderable& obj) noexcept;
		// Update and draw objects
		// Call once, after `begin_draw` and before `end_frame`
		void flush() noexcept;

		// Forces a static objects rebuild
		inline void mark_dirty() noexcept { this->static_dirty = true; }

	private:
		// Identifies a unique draw batch
		struct BatchKey {
			Mesh* mesh;
			u32 vertex_id;
			u32 fragment_id;
			u32 albedo_id;

			bool operator==(const BatchKey& other) const noexcept {
				return this->mesh == other.mesh &&
					this->vertex_id == other.vertex_id &&
					this->fragment_id == other.fragment_id &&
					this->albedo_id == other.albedo_id;
			}
		};

		struct BatchKeyHash {
			size_t operator()(const BatchKey& k) const noexcept {
				size_t seed = 0;
				hash::combine(seed, std::hash<Mesh*>()(k.mesh));
				hash::combine(seed, std::hash<u32>()(k.vertex_id));
				hash::combine(seed, std::hash<u32>()(k.fragment_id));
				hash::combine(seed, std::hash<u32>()(k.albedo_id));
				return seed;
			}
		};

		struct Batch {
			Mesh* mesh = nullptr;
			const Material* material = nullptr;
			AABB aabb;

			// Per-instance GPU data
			std::vector<Renderable::InstanceData> gpu_data;
			// Object owning each entry in 'gpu_data', same index.
			// Only used for static batches
			std::vector<Renderable*> owners;
			// Dangerous if pointer becomes dangling

			// Index of this batch on instances
			u32 instance_index = 0;
			// Result of the last frustum test
			bool visible = true;

			inline size_t push_dynamic(const Renderable::InstanceData& d) noexcept {
				this->gpu_data.push_back(d);
				return this->gpu_data.size() - 1;
			}
			
			inline size_t push_static(const Renderable::InstanceData& d, Renderable* owner) noexcept {
				this->gpu_data.push_back(d);
				this->owners.push_back(owner);
				return this->gpu_data.size() - 1;
			}
		};

	private:
		// Where a specific static object's instance data lives, so a
		// transform change can be patched in-placee without a full rebuild
		struct SlotLocation {
			Batch* batch;
			size_t index; // slot in 'gpu_data'
		};

		// Tracks a single camera's last known position and forward
		struct CameraHistory {
			const Camera* camera = nullptr;
			vec3<float> position;
			vec3<float> forward;
		};

	// Objects
	private:
		using BatchTable = std::unordered_map<BatchKey, Batch, BatchKeyHash>;

		BatchTable static_batches;
		BatchTable dynamic_batches;
		// Maps a static object to where its instance lives inside 'static_batches'
		std::unordered_map<Renderable*, std::vector<SlotLocation>> static_lookup;
		// Flattened static instance data
		std::vector<Renderable::InstanceData> static_instances;
	
		// All objects currently registered as static
		std::vector<Renderable*> static_objs;
		// Static objects that transform changed last frame
		std::vector<Renderable*> dirty_queue;

	// Camera
	private:
		std::vector<CameraHistory> camera_history;
		glm::mat4 cached_view;
		glm::mat4 cached_proj;

	// GPU
	private:
		UniformBuffer ubo_camera;
		ShaderStorageBuffer ssbo_objs;
		ShaderStorageBuffer ssbo_lights;
		ShaderStorageBuffer ssbo_glyphs;
		ProgramPipeline ppipeline;
		Frustum frustum;

	private:
		int pass_index = -1;
		size_t frame_index = 0;

	// Flags
	private:
		// Set when 'static_objs' changed membership, or an object's BatchKey changed
		bool static_dirty = true;
		// Set true when `flaten_persistent` ran. `draw_persistent` check when must
		// test visibility for every batch
		bool static_rebuilt_this_frame = false;
		// Wheter the current pass's camera moved since its own last use.
		// Sourced from 'CameraHistory'
		bool camera_dirty = true;
		// Set by `draw_persistent`
		bool static_included = false;

	private:
		// Resolves the batch a submesh belongs to
		Batch& resolve_batch(const Model::SubMesh& sub, BatchTable& table) noexcept;
		// Inserts a static object into 'static_batches'
		void insert_static(Renderable& obj) noexcept;
		// Inserts a dynamic object into 'dynamic_batches'
		void insert_dynamic(Renderable& obj) noexcept;
		// Rebuilds 'static_instances' from all persistent batches and assigns each batch's 'instance_index'
		// (its offset into that buffer). Also recomputes each batch's AABB from its owners.
		// Called only when 'persistent_dirty' is set
		void flatten_persistent() noexcept;
		// Incremental patch, only objects queued in 'dirty_queue'
		void patch_dirty() noexcept;
};

} // namespace floyd
