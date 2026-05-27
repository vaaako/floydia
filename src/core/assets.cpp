#include "floydia/core/assets.hpp"

#include "floydia/gpu/vertexlayout.hpp"
#include "floydia/geometry/vertex.hpp"
#include "floydia/helpers/hash.hpp"
#include <stdexcept>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace floyd {

Assets::Assets() noexcept {
	u8 white[] = { 255, 255, 255, 255 }; // 1x1 RGBA white
	u8 blackpurple[16] = {
		// Row 0
		0, 0, 0, 255,  255, 0, 255, 255, // Purple, Black
		// Row 1
		255, 0, 255, 255,  0, 0, 0, 255 // Black, Purple
	}; // Here to avoid Error log when library starts
	this->textures[hash::of("d_white")]    = { std::make_shared<Texture>(white, 1, 1), "[DEFAULT_TEXTURE]" };
	this->textures[hash::of("d_notfound")] = { std::make_shared<Texture>(blackpurple, 2, 2), "[DEFAULT_TEXTURE]" };

	this->defaults.PROG_VERT_3D   = this->load_program(Shaders::DEFAULT_VERTEX,    nullptr);
	this->defaults.PROG_FRAG_3D   = this->load_program(nullptr, Shaders::DEFAULT_FRAGMENT);
	this->defaults.PROG_VERT_2D   = this->load_program(Shaders::DEFAULT_VERTEX_2D, nullptr);
	this->defaults.PROG_FRAG_2D   = this->load_program(nullptr, Shaders::DEFAULT_FRAGMENT_2D);
	this->defaults.PROG_VERT_TEXT = this->load_program(Shaders::DEFAULT_VERTEX_TEXT, nullptr);

	this->defaults.MAT_3D = this->load_material(this->defaults.PROG_VERT_3D, this->defaults.PROG_FRAG_3D);
	this->defaults.MAT_2D = this->load_material(this->defaults.PROG_VERT_2D, this->defaults.PROG_FRAG_2D);
}

std::shared_ptr<Texture> Assets::load_texture(const char* path) {
	if(path == nullptr) throw std::invalid_argument("Texture path is null");

	const size_t hash = hash::of(path);
	std::shared_ptr<Texture> tex = this->load<Texture>(hash);
	if(tex != nullptr) return tex;

	tex = std::make_shared<Texture>(path);
	this->textures[hash] = { tex, path };
	return tex;
}

std::shared_ptr<Texture> Assets::load_texture(u8* data, const u32 width, const u32 height, const u8 channels, const char* name) {
	if(data == nullptr) throw std::invalid_argument("Texture data is null");

	const size_t hash = hash::of(data);
	std::shared_ptr<Texture> tex = this->load<Texture>(hash);
	if(tex != nullptr) return tex;

	tex = std::make_shared<Texture>(data, width, height, channels);
	this->textures[hash] = { tex, name };
	return tex;
}

std::shared_ptr<Text> Assets::load_font(const char* path, const u32 size) {
	if(path == nullptr) throw std::invalid_argument("Font path is null");
	const size_t hash = hash::of(path);
	std::shared_ptr<Text> text = this->load<Text>(hash);
	if(text) return text;

	text = std::make_shared<Text>(path, size);
	this->texts[hash] = text;
	return text;
}

std::shared_ptr<ShaderProgram> Assets::load_program(const char* vertex, const char* fragment) {
	// TODO: format shader if custom goes here
	if(vertex == nullptr && fragment == nullptr) throw std::invalid_argument("Vertex and Fragment Shader source are null");

	// Hash the source content — pointer alone is not stable
	size_t hash = 0;
	if(vertex) hash::combine(hash, std::string_view(vertex));
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
	const size_t key = this->material_hash(vertex, fragment);
	std::shared_ptr<Material> material = this->load<Material>(key);
	if(material != nullptr) return material;

	material = std::make_shared<Material>(vertex, fragment);
	this->materials[key] = material;
	return material;
}

std::shared_ptr<Model> Assets::load_model(const char* path) {
	// Check cache
	const size_t key = hash::of(path);
	std::shared_ptr<Model> model = this->load<Model>(key);
	if(model != nullptr) return model;
	model = std::make_shared<Model>();

	// Data containers
	tinyobj::attrib_t attrib; // Mesh information
	std::vector<tinyobj::shape_t> shapes; // Mesh shapes
	std::vector<tinyobj::material_t> materials; // Mesh materials

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
		path, string::base_dir(path).c_str(), true); // path, mtl_basedir, triangulate
	if(!err.empty()) logger::log(logger::Warning, "%s", err.c_str());
	if(!ret) throw std::runtime_error(string::format("Failed to load \"%s\": %s", path, err.c_str()));


	// Each shape = one SubMesh
	for (const tinyobj::shape_t& shape : shapes) {
		std::vector<Vertex> vertices;
		std::vector<u32> indices;
		std::unordered_map<size_t, u32> uniq_verts; // dedup vertices
		
		for(const tinyobj::index_t& idx : shape.mesh.indices) {
			Vertex v;

			v.pos = {
				attrib.vertices[3 * idx.vertex_index + 0],
				attrib.vertices[3 * idx.vertex_index + 1],
				attrib.vertices[3 * idx.vertex_index + 2]
			};

			if(idx.normal_index >= 0) {
				v.normal = {
					attrib.normals[3 * idx.normal_index + 0],
					attrib.normals[3 * idx.normal_index + 1],
					attrib.normals[3 * idx.normal_index + 2]
				};
			}

			if(idx.texcoord_index >= 0) {
				v.uv = {
					attrib.texcoords[2 * idx.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] // flip Y
				};
			}

			// Dedup
			size_t h = 0;
			hash::combine(h, idx.vertex_index);
			hash::combine(h, idx.normal_index);
			hash::combine(h, idx.texcoord_index);

			auto it = uniq_verts.find(h);
			if(it != uniq_verts.end()) {
				// Duplicated
				indices.push_back(it->second);
			} else {
				u32 i = (u32)vertices.size();
				vertices.push_back(v);
				indices.push_back(i);
				uniq_verts[h] = i;
			}
		}

		// Build material from .mtl
		std::shared_ptr<MaterialInstance> matinst = std::make_shared<MaterialInstance>(
			this->defaults.MAT_3D,
			this->load<Texture>(hash::of("d_white"))
		);

		// First material index for this shape
		if(!shape.mesh.material_ids.empty()) {
			int mat_id = shape.mesh.material_ids[0];
			if(mat_id >= 0 && mat_id < (int)materials.size()) {
				const tinyobj::material_t& mat = materials[mat_id];
				if(!mat.diffuse_texname.empty()) {
					const std::string texpath = string::base_dir(path) + mat.diffuse_texname;
					matinst->albedo = this->load_texture(texpath.c_str());
				}
				// Metallic and roughness
				matinst->metallic = mat.metallic;
				matinst->roughness = mat.roughness;
			}
		}

		// Since the Mesh uses Vertex
		// the minimum size is 32 bytes
		// so it has to layout to the three attributes
		VertexLayout layout;
		layout.add<float>(3);
		layout.add<float>(3);
		layout.add<float>(2);
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vertices, indices, layout);
		model->add_submesh(mesh, matinst);
	}

	this->models[key] = model;
	return model;
}

std::shared_ptr<Mesh> Assets::load_quad_mesh() noexcept {
	static std::shared_ptr<Mesh> mesh = []() {
		const std::vector<Vertex2D> vertices = {
			// positions                // texture coords
			{ { -0.5f, -0.5f,  0.5f },  { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f,  0.5f },  { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f,  0.5f },  { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f,  0.5f },  { 0.0f, 1.0f } }
		};

		const std::vector<u8> indices = {
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


std::shared_ptr<Mesh> Assets::load_quad3d_mesh() noexcept {
	static std::shared_ptr<Mesh> mesh = []() {
		const std::vector<Vertex> vertices = {
			// positions               // normals            // texture coords
			{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }
		};
		const std::vector<u8> indices = { 0, 1, 2, 2, 3, 0 };

		VertexLayout layout;
		layout.add<float>(3); // position
		layout.add<float>(3); // normals
		layout.add<float>(2); // texuv
		return std::make_shared<Mesh>(vertices, indices, layout);
	}();
	return mesh;
}

std::shared_ptr<Mesh> Assets::load_cube_mesh() noexcept {
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

		std::vector<u8> indices = {
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


size_t Assets::material_hash(const std::shared_ptr<ShaderProgram>& vertex, const std::shared_ptr<ShaderProgram>& fragment) const noexcept {
	size_t seed = 0;
	hash::combine(seed, vertex->id());
	hash::combine(seed, fragment->id());
	return seed;
}

} // namespace floyd
