#pragma once

#include "floydia/rendering/model.hpp"
#include "floydia/rendering/object.hpp"

#include <memory>

namespace floyd {

// NOTE: Script is on Renderable so it can access Material

// Renderable object
class Renderable : public Object {
	public:
		struct alignas(16) InstanceData {
			glm::mat4 model;
			glm::vec4 color;
		};

	public:
		// Visible on Frustum Culling
		bool visible = true;
		// Set by Renderer. Tells if a Object is persistent on Renderer
		bool is_persistent = false;
		// Set by Renderer. Just to avoid double push on 'dirty_queue'
		bool is_dirty_queued = false;
		// Set by Renderer. Index in renderer
		size_t index = SIZE_MAX;
		// Set by Renderer. If persistent object, position inside batch for rebuild
		u32 persistent_slot = 0;

	public:
		Renderable(const std::shared_ptr<Model>& model) noexcept;
		virtual ~Renderable() = default;

		// Returns world AABB
		virtual AABB world_aabb() noexcept;

		// Returns Model class
		inline Model* model() noexcept { return this->_model.get(); }
		// Returns Model class
		inline const Model* model() const noexcept { return this->_model.get(); }
		// How many meshes are inside the model
		inline size_t mesh_count() const { return this->_model->meshes().size(); }
		// The material of the first mesh.
		// Returns 'nullptr' if no mesh
		inline MaterialInstance* material() noexcept {
			if(this->_model->meshes().empty()) { return nullptr; }
			return this->_model->meshes()[0].material.get();
		}
		// The material of the first mesh.
		// Returns 'nullptr' if no mesh
		inline const MaterialInstance* material() const noexcept {
			if(this->_model->meshes().empty()) { return nullptr; }
			return this->_model->meshes()[0].material.get();
		}
		// The material of the desired mesh.
		// Returns 'nullptr' if no mesh
		inline MaterialInstance* material(const size_t index) noexcept { return this->_model->meshes().at(index).material.get(); }
		// The material of the desired mesh.
		// Returns 'nullptr' if no mesh
		inline const MaterialInstance* material(const size_t index) const noexcept { return this->_model->meshes().at(index).material.get(); }

		// Set the same texture for all submeshes
		void set_albedo_all(const std::shared_ptr<Texture>& tex) noexcept;
	
	protected:
		AABB _world_aabb;
		std::shared_ptr<Model> _model;
};

} // namespace floyd
