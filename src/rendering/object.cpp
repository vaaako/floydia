#include "floydia/rendering/object.hpp"

namespace floyd {

void Object::update_scripts(const float dt) noexcept {
	for(auto& s : this->scripts) s->update(dt);
}

} // namespace floyd
