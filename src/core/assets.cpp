#include <floydia/core/assets.hpp>

#include <floydia/gpu/vertexlayout.hpp>
#include <floydia/geometry/vertex.hpp>

namespace floyd {

Assets::Assets() noexcept {
	// Shader Programs
	this->emplace_program("default", Shaders::DEFAULT_VERTEX, Shaders::DEFAULT_FRAGMENT);
	this->emplace_program("default2d", Shaders::DEFAULT_VERTEX_2D, Shaders::DEFAULT_FRAGMENT);
	// Material
	this->materials["default"] = std::make_shared<Material>(
		this->emplace_program("default_vertex", Shaders::DEFAULT_VERTEX, nullptr),
		this->emplace_program("default_fragment", nullptr, Shaders::DEFAULT_FRAGMENT)
	);
	this->materials["default2d"] = std::make_shared<Material>(
		this->emplace_program("default_vertex_2d", Shaders::DEFAULT_VERTEX_2D, nullptr),
		this->get_program("default_fragment")
	);
	this->materials["font_material"] = std::make_shared<Material>(
		this->emplace_program("default_vertex_2d", Shaders::DEFAULT_VERTEX_2D, nullptr),
		this->emplace_program("text_fragment", nullptr, Shaders::TEXT_FRAGMENT)
	);
	// Meshes
	this->meshes["cube"] = this->make_cube_mesh();
	this->meshes["quad"] = this->make_quad_mesh();
}

std::shared_ptr<ShaderProgram> Assets::emplace_program(const std::string& key, const char* vertex, const char* fragment) {
	// TODO: format shader if custom goes here
	if(vertex == nullptr && fragment == nullptr) throw std::invalid_argument("Vertex and Fragment Shader source are null");

	// Check if program is cached
	std::shared_ptr<ShaderProgram> program = this->get_program(key);
	if(program != nullptr) return program;

	// Create program
	const bool separable = ((vertex != nullptr) ^ (fragment != nullptr)); // Decide if must be built to Program Pipeline
	program = std::make_shared<ShaderProgram>();
	program->set_separable(separable);
	if(vertex != nullptr) {
		Shader vs = Shader(vertex, Shader::Vertex);
		program->attach(vs);
	}
	if(fragment != nullptr) {
		Shader fs = Shader(fragment, Shader::Fragment);
		program->attach(fs);
	}
	program->link();
	this->programs[key] = program;
	return program;
}

std::shared_ptr<Mesh> Assets::make_cube_mesh() noexcept {
	static std::shared_ptr<Mesh> mesh = []() {
		std::vector<Vertex> vertices = {
			// positions                // normals               // texture coords
			// Front face (z = 0.5) - Normal: {0, 0, 1}
			{ { -0.5f, -0.5f,  0.5f },  { 0.0f,  0.0f,  1.0f },  { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f,  0.5f },  { 0.0f,  0.0f,  1.0f },  { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f,  0.5f },  { 0.0f,  0.0f,  1.0f },  { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f,  0.5f },  { 0.0f,  0.0f,  1.0f },  { 0.0f, 1.0f } },

			// Back face (z = -0.5) - Normal: {0, 0, -1}
			{ { -0.5f, -0.5f, -0.5f },  { 0.0f,  0.0f, -1.0f },  { 1.0f, 0.0f } },
			{ { -0.5f,  0.5f, -0.5f },  { 0.0f,  0.0f, -1.0f },  { 1.0f, 1.0f } },
			{ {  0.5f,  0.5f, -0.5f },  { 0.0f,  0.0f, -1.0f },  { 0.0f, 1.0f } },
			{ {  0.5f, -0.5f, -0.5f },  { 0.0f,  0.0f, -1.0f },  { 0.0f, 0.0f } },

			// Left face (x = -0.5) - Normal: {-1, 0, 0}
			{ { -0.5f, -0.5f, -0.5f },  { -1.0f, 0.0f,  0.0f },  { 0.0f, 0.0f } },
			{ { -0.5f, -0.5f,  0.5f },  { -1.0f, 0.0f,  0.0f },  { 1.0f, 0.0f } },
			{ { -0.5f,  0.5f,  0.5f },  { -1.0f, 0.0f,  0.0f },  { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f, -0.5f },  { -1.0f, 0.0f,  0.0f },  { 0.0f, 1.0f } },

			// Right face (x = 0.5) - Normal: {1, 0, 0}
			{ {  0.5f, -0.5f,  0.5f },  { 1.0f,  0.0f,  0.0f },  { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, -0.5f },  { 1.0f,  0.0f,  0.0f },  { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, -0.5f },  { 1.0f,  0.0f,  0.0f },  { 1.0f, 1.0f } },
			{ {  0.5f,  0.5f,  0.5f },  { 1.0f,  0.0f,  0.0f },  { 0.0f, 1.0f } },

			// Bottom face (y = -0.5) - Normal: {0, -1, 0}
			{ { -0.5f, -0.5f, -0.5f },  { 0.0f, -1.0f,  0.0f },  { 0.0f, 1.0f } },
			{ {  0.5f, -0.5f, -0.5f },  { 0.0f, -1.0f,  0.0f },  { 1.0f, 1.0f } },
			{ {  0.5f, -0.5f,  0.5f },  { 0.0f, -1.0f,  0.0f },  { 1.0f, 0.0f } },
			{ { -0.5f, -0.5f,  0.5f },  { 0.0f, -1.0f,  0.0f },  { 0.0f, 0.0f } },

			// Top face (y = 0.5) - Normal: {0, 1, 0}
			{ { -0.5f,  0.5f,  0.5f },  { 0.0f,  1.0f,  0.0f },  { 0.0f, 1.0f } },
			{ {  0.5f,  0.5f,  0.5f },  { 0.0f,  1.0f,  0.0f },  { 1.0f, 1.0f } },
			{ {  0.5f,  0.5f, -0.5f },  { 0.0f,  1.0f,  0.0f },  { 1.0f, 0.0f } },
			{ { -0.5f,  0.5f, -0.5f },  { 0.0f,  1.0f,  0.0f },  { 0.0f, 0.0f } }
		};

		std::vector<uint8> indices = {
			// Front face
			0, 1, 2,
			2, 3, 0,
			// Back face
			4, 5, 6,
			6, 7, 4,
			// Left face
			8, 9, 10,
			10, 11, 8,
			// Right face
			12, 13, 14,
			14, 15, 12,
			// Bottom face
			16, 17, 18,
			18, 19, 16,
			// Top face
			20, 21, 22,
			22, 23, 20
		};

		VertexLayout layout;
		layout.add<float>(3); // position
		layout.add<float>(3); // normal
		layout.add<float>(2); // texuv
		return std::make_shared<Mesh>(vertices, indices, layout);
	}();
	return mesh;
}

std::shared_ptr<Mesh> Assets::make_quad_mesh() noexcept {
	static std::shared_ptr<Mesh> mesh = []() {
		std::vector<Vertex2D> vertices = {
			// positions                // texture coords
			{ { -0.5f, -0.5f,  0.5f },  { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f,  0.5f },  { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f,  0.5f },  { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f,  0.5f },  { 0.0f, 1.0f } },
		};

		std::vector<uint8> indices = {
			0, 1, 2,
			2, 3, 0,
		};

		VertexLayout layout;
		layout.add<float>(3); // position
		layout.add<float>(2); // texuv
		return std::make_shared<Mesh>(vertices, indices, layout);
	}();
	return mesh;
}

} // namespace floyd
