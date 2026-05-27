#include "floydia/rendering/script.hpp"
#include "floydia/rendering/renderable.hpp"

namespace floyd {

Renderable* Script::as_renderable() const noexcept { return dynamic_cast<Renderable*>(owner); }

} // namespace floyd
