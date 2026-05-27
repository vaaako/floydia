#pragma once

namespace floyd {

class Object;
class Renderable;

// Attachable script for Renderable
struct Script {
	Object* owner = nullptr;
	virtual void on_attach() noexcept {}
	virtual void update(const float dt) noexcept { (void)dt; }
	virtual ~Script() noexcept = default;
	// Cast owner to Renderable object
	Renderable* as_renderable() const noexcept;
};

// NOTE: methods are not raw virtual so user is not obligated to define it

} // namespace floyd
