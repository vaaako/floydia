#include <floydia/geometry/billboard.hpp>

#include "floydia/core/core.hpp"
#include <floydia/material/materialinstance.hpp>

namespace floyd {

Billboard::Billboard() noexcept : Renderable(Billboard::create_model()) {}

glm::mat4 Billboard::final_matrix(const glm::mat4& view) const noexcept {
	const glm::mat4 model = this->transform.model_matrix();
	glm::mat4 mv    = view * model;

	if(this->type == Billboard::Full) {
		mv[0] = glm::vec4(glm::length(glm::vec3(model[0])), 0, 0, 0);
		mv[1] = glm::vec4(0, glm::length(glm::vec3(model[1])), 0, 0);
		mv[2] = glm::vec4(0, 0, glm::length(glm::vec3(model[2])), 0);
	}

	if(this->type == Billboard::Cylindrical) {
		mv[0] = glm::vec4(glm::length(glm::vec3(model[0])), 0, 0, 0);
		mv[2] = glm::vec4(0, 0, glm::length(glm::vec3(model[2])), 0);
	}

	return glm::inverse(view) * mv; // Expensive
}

std::shared_ptr<Model> Billboard::create_model() noexcept {
	std::shared_ptr<Mesh> mesh = assets().load_quad3d_mesh();
	std::shared_ptr<MaterialInstance> matinst = std::make_shared<MaterialInstance>(
		assets().defaults.MAT_3D,
		assets().load<Texture>(hash::of("d_white"))
	);
	std::shared_ptr<Model> model = std::make_shared<Model>();
	model->add_submesh(mesh, matinst); // use default material
	return model;
}

} // namespace floyd
