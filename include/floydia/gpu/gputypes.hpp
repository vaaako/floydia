#pragma once

#include <floydia/types.hpp>

namespace gpu {
	struct CameraBlock {
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct InstanceData {
		glm::mat4 model;
	};
}
