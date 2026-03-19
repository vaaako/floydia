#pragma once

#include <floydia/types.hpp>
#include <floydia/camera/camera.hpp>
#include <floydia/gfx/renderable.hpp>
#include <floydia/gfx/mesh.hpp>
#include <floydia/gfx/material.hpp>
#include <floydia/gpu/uniformbuffer.hpp>
#include <floydia/gpu/ssbo.hpp>

#include <vector>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

namespace floyd {

class Renderer final {
	public:
		Renderer() noexcept;
		~Renderer() = default;

		// Initialize OpenGL
		void init() noexcept;

		// Clear the screen
		void clear() const noexcept;
		// Changes the clear color
		void set_clear_color(const vec4<uint8>& color) noexcept;

		// Clear queue and update Uniform Buffer
		void begin_draw(const Camera& camera) noexcept;
		// Submit object to the queue
		void push(const Renderable& object) noexcept;
		// End frame
		void flush();

	private:
		struct DrawBatch {
			Mesh* mesh;
			Material* material;
			// Number of instances for this mesh
			uint32 instance_count;
			// Start index in SSBO
			uint32 instance_index;
		};

		struct alignas(16) CameraData {
			glm::mat4 view;
			glm::mat4 proj;
		};

		struct alignas(16) InstanceData {
			glm::mat4 model;
			glm::vec4 color;
		};

	private:

		std::vector<DrawBatch> batches;
		std::vector<InstanceData> instances;
		
		// In order for batches to work, instances (SSBO data) must be accessed in batch order.
		// Instead of duplicating InstanceData to make them contiguous,
		// this vector stores an indirection buffer (instance_indices).
		// This buffer maps each draw instance to its original instance in the SSBO.
		// A second SSBO is used to send these indices to the shader
		std::vector<uint32> instance_indices;

		UniformBuffer ubo_camera;
		ShaderStorageBuffer ssbo_instance;
		ShaderStorageBuffer ssbo_instance_indices;

		float clear_color[4] = { 0.1f, 0.1f, 0.1f, 0.1f };

		// TODO: add resize for SSBO
		// constexpr static uint32 INST_AMOUNT = 9999;
		constexpr static uint32 INST_AMOUNT = 999999; // debug
};

} // namespace floyd
