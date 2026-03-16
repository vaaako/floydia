#pragma once

#include <floydia/gfx/model.hpp>
#include <floydia/gfx/material.hpp>
#include <floydia/gfx/object.hpp>

#include <memory>

namespace floyd {

// Renderable object
class Renderable : public Object {
	public:
		Renderable(const std::shared_ptr<Model>& model) noexcept;
		virtual ~Renderable() = default;

		// Returns Model class
		inline Model* model() noexcept { return this->_model.get(); }
		// Returns Model class
		inline const Model* model() const noexcept { return this->_model.get(); }

		// How many meshes are inside the model
		inline size_t mesh_count() const { return this->_model->meshes().size(); }

		// The material of the first mesh.
		// Returns 'nullptr' if no mesh
		inline Material* material() noexcept {
			if(this->_model->meshes().empty()) { return nullptr; }
			return this->_model->meshes()[0].material.get();
		}
		// The material of the first mesh.
		// Returns 'nullptr' if no mesh
		inline const Material* material() const noexcept {
			if(this->_model->meshes().empty()) { return nullptr; }
			return this->_model->meshes()[0].material.get();
		}
		// The material of the desired mesh.
		// Returns 'nullptr' if no mesh
		inline Material* material(const size_t index) noexcept { return this->_model->meshes().at(index).material.get(); }
		// The material of the desired mesh.
		// Returns 'nullptr' if no mesh
		inline const Material* material(const size_t index) const noexcept { return this->_model->meshes().at(index).material.get(); }

	protected:
		std::shared_ptr<Model> _model;
};

} // namespace floyd
