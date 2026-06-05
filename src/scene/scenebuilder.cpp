#include "floydia/scene/scenebuilder.hpp"
#include "toml.hpp"

namespace floyd {

// - Save only camera object and user configs later?
// - Save only constant objects?
// - Dont save camera?

SceneBuilder SceneBuilder::load(const char* path, Renderer& renderer) noexcept {
	SceneBuilder scene;
	toml::parse_result root = toml::parse_file(path);

	// Camera
	if(auto cam = root["camera"].as_table()) {
		std::string type = cam->get_as<std::string>("type")->get();
		if(type == "perspective") {
			float fov = cam->get_as<double>("fov")->get(); // it has to be double
			// TODO: test only
			scene.perspective_camera = new PerspectiveCamera(fov, 800.0f, 600.0f);

			if(auto pos = (*cam)["position"].as_array()) {
				scene.perspective_camera->position = {
					pos->get_as<double>(0)->get(),
					pos->get_as<double>(1)->get(),
					pos->get_as<double>(2)->get()
				};
			}
		} else if(type == "orthographic") {
			// TODO: test only
			scene.ortho_camera = new OrthoCamera(800.0f, 600.0f);
		}
	}

	return scene;
}

void SceneBuilder::save(const char* path, Renderer& renderer) noexcept {
	toml::table root;

	// -- Camera
	toml::table cam;
	if(this->perspective_camera) {
		cam.insert("type", "perspective");
		cam.insert("fov", perspective_camera->get_fov());
		cam.insert("position", toml::array{ this->perspective_camera->position.x, this->perspective_camera->position.y, this->perspective_camera->position.z });
	}

	if(this->ortho_camera) {
		cam.insert("type", "orthographic");
		cam.insert("position", toml::array{ this->ortho_camera->position.x, this->ortho_camera->position.y, this->ortho_camera->position.z });
	}
	root.insert("camera", std::move(cam));


	// -- Lights
	toml::array lights_arr;
	auto save_light = [&](const Light::LightData& l, const bool persistent) {
		toml::table lt;

		std::string type = "point";
		if(l.type == Light::Type::Directional) type = "directional";
		else if(l.type == Light::Type::Spot) type = "spot";

		lt.insert("type", type);
		lt.insert("persistent", persistent);
		lt.insert("intensity", l.color.w);
		lt.insert("range", l.range);

		lt.insert("color", toml::array{ l.color.x, l.color.y, l.color.z, 1.0f });
		const vec3<float> pos = l.position;
		lt.insert("position", toml::array{ pos.x, pos.y, pos.z });
		const vec3<float> rot = l.direction;
		lt.insert("direction", toml::array{ rot.x, rot.y, rot.z });

		lights_arr.push_back(std::move(lt));
	};

	for(const Light::LightData& l : renderer.persistent_lights) save_light(l, true);
	// for(const Light::LightData& l : renderer.lights) save_light(l, false);

	root.insert("lights", std::move(lights_arr));

	// -- Objects
	toml::array objs_arr;
	auto save_objs = [&](const Renderable& r, const bool persistent) {
		toml::table ot;

		// Type
		if(dynamic_cast<const Cube*>(&r)) ot.insert("type", "cube");
		else if(dynamic_cast<const Sprite*>(&r)) ot.insert("type", "sprite");

		// Transform
		const vec3<float>& pos = r.transform.position();
		ot.insert("position", toml::array{ pos.x, pos.y, pos.z });
		const vec3<float>& rot = r.transform.euler_degrees();
		ot.insert("rotation", toml::array{ rot.x, rot.y, rot.z });
		const vec3<float>& scl = r.transform.scale();
		ot.insert("scale", toml::array{ scl.x, scl.y, scl.z });

		// Material
		const vec4<float>& col = r.color_norm();
		ot.insert("color", toml::array{ col.x, col.y, col.z, col.w });
		if(const MaterialInstance* mat = r.material()) {
			ot.insert("metallic",  mat->metallic);
			ot.insert("roughness", mat->roughness);
			// TODO: texture and shaders
		}
		objs_arr.push_back(std::move(ot));
	};

	for(const Renderable* o : renderer.persistent_objs) save_objs(*o, true);
	// for(const Renderable* o : renderer.pickables) save_objs(*o, false);
	root.insert("objects", std::move(objs_arr));


	// Write to file
	std::ofstream file = std::ofstream(path);
	file << root;
}

} // namespace floyd
