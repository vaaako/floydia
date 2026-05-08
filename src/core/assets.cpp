#include <floydia/core/assets.hpp>

#include <floydia/gpu/vertexlayout.hpp>
#include <floydia/geometry/vertex.hpp>
#include <floydia/helpers/hash.hpp>

namespace floyd {

Assets::Assets() noexcept {
	uint8 white[] = { 255, 255, 255, 255 }; // 1x1 RGBA white
	this->textures[hash::of(std::string_view("d_white"))] = std::make_shared<Texture>(white, 1, 1);
	this->textures[hash::of(std::string_view("d_notfound"))] = std::make_shared<Texture>(nullptr, 2, 2); // Make not found texture

	std::shared_ptr<ShaderProgram> vert_3d   = this->load_program(Shaders::DEFAULT_VERTEX,    nullptr);
	std::shared_ptr<ShaderProgram> vert_2d   = this->load_program(Shaders::DEFAULT_VERTEX_2D, nullptr);
	std::shared_ptr<ShaderProgram> frag_3d   = this->load_program(nullptr, Shaders::DEFAULT_FRAGMENT);
	std::shared_ptr<ShaderProgram> frag_2d   = this->load_program(nullptr, Shaders::DEFAULT_FRAGMENT_2D);
	std::shared_ptr<ShaderProgram> frag_text = this->load_program(nullptr, Shaders::TEXT_FRAGMENT);

	// Default 3D: vert_3d + frag_def + white texture
	std::shared_ptr<Material> mat_3d = std::make_shared<Material>(vert_3d, frag_3d);
	this->materials[this->material_hash(vert_3d, frag_3d)] = mat_3d;

	// Default 2D: vert_2d + frag_def + white texture
	std::shared_ptr<Material> mat_2d = std::make_shared<Material>(vert_2d, frag_2d);
	this->materials[this->material_hash(vert_2d, frag_2d)] = mat_2d;

	// Font: vert_2d + frag_text + white texture
	std::shared_ptr<Material> mat_font = std::make_shared<Material>(vert_2d, frag_text);
	this->materials[this->material_hash(vert_2d, frag_text)] = mat_font;

	this->meshes[hash::of(std::string_view("cube"))] = this->make_cube_mesh();
	this->meshes[hash::of(std::string_view("quad"))] = this->make_quad_mesh();
}

std::shared_ptr<Texture> Assets::load_texture(const char* path) {
	if(path == nullptr) throw std::invalid_argument("Texture path is null");
	
	size_t hash = hash::of(std::string_view(path));
	std::shared_ptr<Texture> tex = this->load<Texture>(hash);
	if(tex) return tex;

	tex = std::make_shared<Texture>(path);
	this->textures[hash] = tex;
	return tex;
}

std::shared_ptr<ShaderProgram> Assets::load_program(const char* vertex, const char* fragment) {
	// TODO: format shader if custom goes here
	if(vertex == nullptr && fragment == nullptr) throw std::invalid_argument("Vertex and Fragment Shader source are null");

	// Hash the source content — pointer alone is not stable
	size_t hash = 0;
	if(vertex)   hash::combine(hash, std::string_view(vertex));
	if(fragment) hash::combine(hash, std::string_view(fragment));

	// Return cached if exists
	std::shared_ptr<ShaderProgram> program = this->load<ShaderProgram>(hash);
	if(program != nullptr) return program;

	// Create program
	const bool separable = ((vertex != nullptr) ^ (fragment != nullptr)); // Decide if must be built to Program Pipeline
	program = std::make_shared<ShaderProgram>();
	program->set_separable(separable);
	if(vertex) {
		Shader vs = Shader(vertex, Shader::Vertex);
		program->attach(vs);
	}
	if(fragment) {
		Shader fs = Shader(fragment, Shader::Fragment);
		program->attach(fs);
	}
	program->link();

	this->programs[hash] = program;
	return program;
}

std::shared_ptr<Material> Assets::load_material(
	const std::shared_ptr<ShaderProgram>& vertex,
	const std::shared_ptr<ShaderProgram>& fragment
) noexcept {
	size_t key = this->material_hash(vertex, fragment);
	std::shared_ptr<Material> material = this->load<Material>(key);
	if (material != nullptr) return material;

	material = std::make_shared<Material>(vertex, fragment);
	this->materials[key] = material;
	return material;
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


size_t Assets::material_hash(const std::shared_ptr<ShaderProgram>& vertex, const std::shared_ptr<ShaderProgram>& fragment) const noexcept {
	size_t seed = 0;
	hash::combine(seed, vertex->id());
	hash::combine(seed, fragment->id());
	return seed;
}

} // namespace floyd
