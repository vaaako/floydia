#include <floydia/gfx/renderable.hpp>

#include <floydia/helpers/log.hpp>

namespace floyd {

Renderable::Renderable(const std::shared_ptr<Model>& model) noexcept
	: _model(std::move(model)) {}

} // namespace floyd
