#pragma once

#include "floydia/rendering/model.hpp"
#include "floydia/rendering/object.hpp"

#include <memory>

namespace floyd {

// NOTE: Script is on Renderable so it can access Material

// Renderable object
class Renderable : public Object {
	friend class Renderer;
	
	public:
		struct alignas(16) InstanceData {
			glm::mat4 model;
			glm::vec4 color;
			float metallic;
			float roughness;
			float _pad[2]; // alignment
		};

	public:
		// Set by Renderer. Index in renderer
		size_t index = SIZE_MAX;

	public:
		Renderable() noexcept = default;
		Renderable(const std::shared_ptr<Model>& model) noexcept;
		virtual ~Renderable() = default;

		// Returns world AABB
		virtual AABB world_aabb() noexcept;

		// Returns Model class
		inline Model* model() noexcept { return this->_model.get(); }
		// Returns Model class
		inline const Model* model() const noexcept { return this->_model.get(); }
		// Set a new model
		inline void set_model(const std::shared_ptr<Model>& model) noexcept { this->_model = model; }
		// How many meshes are inside the model
		inline size_t mesh_count() const { return this->_model->meshes().size(); }

		// Mesh's material (first mesh by default)
		inline Material& material(const size_t index = 0) noexcept { return this->_model->meshes().at(index).material; }
		// Mesh's material (first mesh by default)
		inline const Material& material(const size_t index = 0) const noexcept { return this->_model->meshes().at(index).material; }

		// Set a texture of a mesh (first mesh by default)
		void set_texture(const std::shared_ptr<Texture>& albedo, const size_t index = 0) noexcept;
		// Set the same texture for all meshes
		void set_albedo_all(const std::shared_ptr<Texture>& albedo) noexcept;
	
	private:
		// Set by Renderer. Tells if a object is static
		bool is_persistent = false;
		// Set by Renderer. Tells when a static object's BatchKey changed
		bool needs_rebatch = false;
		bool is_dirty_queued = false;

	protected:
		AABB _world_aabb;
		std::shared_ptr<Model> _model;
};

} // namespace floyd
