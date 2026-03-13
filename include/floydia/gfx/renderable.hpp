#pragma once

#include <floydia/core/renderer.hpp>
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

		inline Model* model() const noexcept {
			return this->_model.get();
		}

		void submit(Renderer& renderer) const noexcept override;

	protected:
		std::shared_ptr<Model> _model;
};

} // namespace floyd
