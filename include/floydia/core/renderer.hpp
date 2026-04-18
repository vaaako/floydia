#pragma once

#include "floydia/camera/frustum.hpp"
#include <floydia/types.hpp>
#include <floydia/camera/camera.hpp>
#include <floydia/rendering/renderable.hpp>
#include <floydia/rendering/mesh.hpp>
#include <floydia/rendering/material.hpp>
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

		// Clear queue and update Uniform Buffer
		void begin_draw(const Camera& camera) noexcept;
		// Submit object to the queue
		void push(Renderable& object) noexcept;
		// End frame
		void flush() noexcept;

	private:
		struct InstanceData; // (to keep data aligment)

		struct DrawBatch {
			Mesh* mesh;
			Material* material;
			std::vector<InstanceData> instances;
			// Start index in SSBO
			uint32 instance_index; // offset into final instance buffer
			// Number of instances for this mesh
			uint32 instance_count;
		};

		struct alignas(16) CameraData {
			glm::mat4 view;
			glm::mat4 proj;
		};

		struct alignas(16) InstanceData {
			glm::mat4 model;
			glm::vec4 color;
		};

		struct BatchKey {
			Mesh* mesh;
			Material* material;
			bool operator==(const BatchKey& other) const noexcept {
				return mesh == other.mesh &&
					material == other.material;
			}
		};

		struct BatchKeyHash {
			std::size_t operator()(const BatchKey& k) const noexcept {
				size_t seed = 0;
				hash::combine(seed, std::hash<Mesh*>()(k.mesh));
				hash::combine(seed, std::hash<Material*>()(k.material));
				return seed;
			}
		};

	private:
		// All batches on the scene
		std::unordered_map<BatchKey, DrawBatch, BatchKeyHash> batches;
		// Index of batch inside the 'ssbo_instance'
		std::vector<InstanceData> instances;

		Frustum frustum;

		// These are pointers so I can initialize when OpenGL is ready
		UniformBuffer ubo_camera;
		ShaderStorageBuffer ssbo_instance;
		ProgramPipeline ppipeline;

		// ShaderStorageBuffer ssbo_instance_indices;
		size_t total_instances = 0;
		float clear_color[4] = { 0.1f, 0.1f, 0.1f, 0.1f };

		uint32 frameindex = 0;
};

} // namespace floyd
