#pragma once

#include <floydia/types.hpp>
#include <floydia/gfx/renderable.hpp>
#include <floydia/gfx/mesh.hpp>
#include <floydia/gfx/material.hpp>

#include <vector>

// https://chatgpt.com/s/t_69ace069dc7c81918d8e4091737441e2
// https://chatgpt.com/s/t_69ace0d2ca88819186f7c6f9f06e386d
//class Renderer {
//private:
//    ShaderStorageBuffer instanceSSBO;

//    std::vector<InstanceData> instanceCPU;

//public:
//    Renderer()
//        : instanceSSBO(sizeof(InstanceData) * 10000, 1) {}
//};

namespace floyd {

class Renderer final {
	public:
		struct DrawCommand {
			const Mesh* mesh;
			const Material* material;
			const glm::mat4 model;
		};

	public:
		Renderer() = default;
		~Renderer();

		void init();
		void begin_frame();
		void end_frame();

		// void draw(const Renderable& object);

		// Submit a object to the renderable queue
		void submit(const DrawCommand& command) noexcept;

	private:
		struct alignas(16) CameraData {
			glm::mat4 view;
			glm::mat4 proj;
		};

		// alignas(??)
		struct alignas(16) InstanceData {
			glm::mat4 model;
		};

		//UniformBuffer cameraUBO;
		//ShaderStorageBuffer instanceSSBO;

		std::vector<DrawCommand> draw_queue;
	
	private:
		//void upload_camera(const Camera& camera);
		void flush();
};

} // namespace floyd
