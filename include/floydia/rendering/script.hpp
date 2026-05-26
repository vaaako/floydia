#pragma once

namespace floyd {

class Renderable;

// Attachable script to Renderable
struct Script {
	Renderable* owner = nullptr;
	virtual void on_attach() noexcept {}
	virtual void update(const float dt) noexcept { (void)dt; }
	virtual ~Script() noexcept = default;
};

// NOTE: methods are not raw virtual so user is not obligated to define it

} // namespace floyd
