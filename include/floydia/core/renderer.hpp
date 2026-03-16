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
		struct DrawCommand {
			const Mesh* mesh;
			const Material* material;
			const glm::mat4 model;
		};

	public:
		Renderer() noexcept;
		~Renderer() = default;

		// Clear queue and update Uniform Buffer
		void begin_frame(Camera& camera) noexcept;
		// End frame
		void flush();
		// Submit a object to the renderable queue
		void submit(const DrawCommand& command) noexcept;
		// Submit object to the queue
		// void draw(const Renderable& object) noexcept;

	private:
		struct alignas(16) CameraData {
			glm::mat4 view;
			glm::mat4 proj;
		};

		// alignas(??)
		struct alignas(16) InstanceData {
			glm::mat4 model;
		};

		std::vector<DrawCommand> draw_queue;
		std::vector<InstanceData> instances;

		UniformBuffer ubo_camera;
		ShaderStorageBuffer ssbo_instance;
};

} // namespace floyd
