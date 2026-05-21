#include <floydia/rendering/renderable.hpp>

#include <floydia/helpers/logger.hpp>
#include <floydia/core/core.hpp>

namespace floyd {

Renderable::Renderable(const std::shared_ptr<Model>& model) noexcept
	: _model(std::move(model)) {}


glm::mat4 Renderable::final_matrix(const glm::mat4& view) const noexcept {
	(void)(view);
	return this->transform.model_matrix();
}

} // namespace floyd
