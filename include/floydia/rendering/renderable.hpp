#pragma once

#include "floydia/rendering/model.hpp"
#include "floydia/rendering/script.hpp"
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
		bool is_persitent = false; // Used by Renderer to mark persistent objects

	public:
		Renderable(const std::shared_ptr<Model>& model) noexcept;
		Renderable(const Renderable& other) noexcept;
		virtual ~Renderable() = default;

		// Attach a script
		template <typename T, typename... Args>
		T* attach_script(Args&&... args) noexcept;
		// Update all attached scripts
		void update_scripts(const float dt) noexcept;

		// Overridable for Redenderable objects that need aditional math on 'model_matrix'
		// Remember to update 'world_aabb'
		virtual glm::mat4 final_matrix(const glm::mat4& view) const noexcept;
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
	
	protected:
		AABB _world_aabb;
		std::shared_ptr<Model> _model;
		std::vector<std::unique_ptr<Script>> scripts;
};

template <typename T, typename... Args>
T* Renderable::attach_script(Args&&... args) noexcept {
	static_assert(std::is_base_of_v<Script, T>, "T must derive from Script");
	
	std::unique_ptr<T> s = std::make_unique<T>(std::forward<Args>(args)...);
	s->owner = this;
	s->on_attach(); // Trigger

	T* ptr = s.get();
	this->scripts.push_back(std::move(s));
	return ptr;
}

} // namespace floyd
