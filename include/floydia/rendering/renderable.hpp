#pragma once

#include <floydia/rendering/model.hpp>
#include <floydia/rendering/object.hpp>
#include <floydia/material/material.hpp>

#include <memory>

namespace floyd {

// Renderable object
class Renderable : public Object {
	public:
		// Fast test
		AABB world_aabb;
		// Visible on Frustum Culling
		bool visible = true;
	public:
		Renderable(const std::shared_ptr<Model>& model) noexcept;
		virtual ~Renderable() = default;

		// Rebuild `world_aabb`
		inline void rebuild_world_aabb() noexcept { this->world_aabb = this->_model->aabb.to_world(this->transform.model_matrix()); }
		// How many meshes are inside the model
		inline size_t mesh_count() const { return this->_model->meshes().size(); }
		// Overridable for Redenderable objects that need aditional math on `model_matrix`
		inline virtual glm::mat4 final_matrix(const glm::mat4& view) const noexcept { return this->transform.model_matrix(); }

		// Returns Model class
		inline Model* model() noexcept { return this->_model.get(); }
		// Returns Model class
		inline const Model* model() const noexcept { return this->_model.get(); }
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
	
	protected:
		std::shared_ptr<Model> _model;
};

} // namespace floyd
