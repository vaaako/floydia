#pragma once

#include <floydia/types.hpp>
#include <floydia/camera/camera.hpp>
#include <floydia/gfx/renderable.hpp>
#include <floydia/gfx/mesh.hpp>
#include <floydia/gfx/material.hpp>
#include <floydia/gpu/uniformbuffer.hpp>
#include <floydia/gpu/ssbo.hpp>

#include <vector>

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
		// Not const so it can be sorted
		struct DrawCommand {
			Mesh* mesh;
			Material* material;
			glm::mat4 model;
		};

		struct alignas(16) CameraData {
			glm::mat4 view;
			glm::mat4 proj;
		};

		struct alignas(16) InstanceData {
			glm::mat4 model;
		};

	private:

		std::vector<DrawCommand> draw_queue;
		std::vector<InstanceData> instances;

		UniformBuffer ubo_camera;
		ShaderStorageBuffer ssbo_instance;

		float clear_color[4] = { 0.1f, 0.1f, 0.1f, 0.1f };
		constexpr static uint32 INST_AMOUNT = 1000;
};

} // namespace floyd
