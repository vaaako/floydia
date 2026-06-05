#pragma once

#include "floydia/camera/orthocamera.hpp"
#include "floydia/camera/perspectivecamera.hpp"
#include "floydia/core/renderer.hpp"
#include "floydia/rendering/light.hpp"
#include "floydia/rendering/renderable.hpp"
#include <memory>
#include <vector>

namespace floyd {

struct SceneBuilder {
	std::vector<std::unique_ptr<Renderable>> persistent;
	std::vector<std::unique_ptr<Renderable>> dynamics;
	std::vector<std::unique_ptr<Light>> dynamic_lights;
	std::vector<std::unique_ptr<Light>> persistent_lights;

	PerspectiveCamera* perspective_camera;
	OrthoCamera* ortho_camera;

	// Load an existing scene
	[[nodiscard]] static SceneBuilder load(const char* path, Renderer& renderer) noexcept;
	// Save current scene to a file
	void save(const char* path, Renderer& renderer) noexcept;
};


} // namespace floyd
