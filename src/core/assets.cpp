#include "floydia/core/assets.hpp"

#include "floydia/gpu/vertexlayout.hpp"
#include "floydia/geometry/vertex.hpp"
#include "floydia/helpers/hash.hpp"
#include "floydia/helpers/logger.hpp"
#include "floydia/rendering/image.hpp"
#include <stdexcept>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace floyd {

Assets::Assets() noexcept {
	u8 white[] = { 255, 255, 255, 255 }; // 1x1 RGBA white
	u8 notfound[16]; // Remove 'const'
	std::memcpy(notfound, Image::FALLBACK_IMAGE, sizeof(Image::FALLBACK_IMAGE));
	this->textures[hash::of("d_white")]    = { std::make_shared<Texture>(white, 1, 1), "[DEFAULT_TEXTURE]" };
	this->textures[hash::of("d_notfound")] = { std::make_shared<Texture>(notfound, 2, 2), "[DEFAULT_TEXTURE]" };

	this->progs.PROG_VERT_3D   = this->load_program(Shaders::DEFAULT_VERTEX, nullptr);
	this->progs.PROG_FRAG_3D   = this->load_program(nullptr, Shaders::DEFAULT_FRAGMENT);
	this->progs.PROG_VERT_2D   = this->load_program(Shaders::DEFAULT_VERTEX_2D, nullptr);
	this->progs.PROG_FRAG_2D   = this->load_program(nullptr, Shaders::DEFAULT_FRAGMENT_2D);
	this->progs.PROG_VERT_TEXT = this->load_program(Shaders::DEFAULT_VERTEX_TEXT, nullptr);
}

std::shared_ptr<Texture> Assets::load_texture(const char* path) noexcept {
	if(path == nullptr) {
		TRACELOG(logger::Error, "Texture path is null");
		return this->load<Texture>(hash::of("d_notfound"));
	}
	const size_t hash = hash::of(path);

	auto it = this->textures.find(hash);
	if(it != this->textures.end()) return it->second.texture;

	std::shared_ptr<Texture> tex = std::make_shared<Texture>(path);
	this->textures[hash] = { tex, path };
	return tex;
}

std::shared_ptr<Texture> Assets::load_texture(u8* data, const u32 width, const u32 height, const u8 channels, const char* name) noexcept {
	if(data == nullptr) {
		TRACELOG(logger::Error, "Texture data is null");
		return this->load<Texture>(hash::of("d_notfound"));
	}

	// Hash the actual pixel content, not the pointer
	const size_t byte_count = static_cast<size_t>(width) * height * channels;
	const size_t hash = hash::of(data, byte_count);

	auto it = this->textures.find(hash);
	if(it != this->textures.end()) return it->second.texture;

	std::shared_ptr<Texture> tex = std::make_shared<Texture>(data, width, height, channels);
	this->textures[hash] = { tex, name };
	return tex;
}

std::shared_ptr<Texture> Assets::load_spritesheet_cell(const std::shared_ptr<Texture>& sheet,
	const u32 frame_width, const u32 frame_height, const u32 index) {

	if(sheet == nullptr) {
		TRACELOG(logger::Error, "Spritesheet texture is null");
		return this->load<Texture>(hash::of("d_notfound"));
	}

	// Hash this cell
	size_t hash = hash::of(sheet->id());
	hash::combine(hash, frame_width);
	hash::combine(hash, frame_height);
	hash::combine(hash, index);

	auto it = this->textures.find(hash);
	if(it != this->textures.end()) return it->second.texture;

	const u32 columns = sheet->width() / frame_width;
	const u32 col = index % columns;
	const u32 row = index / columns;
	const u32 x = col * frame_width;
	const u32 y = row * frame_height;
	const u8 channels = sheet->channels();
	// Load with the same format
	const GLenum format = opengl::channel_to_format(channels, false);

	std::vector<u8> cell = std::vector<u8>(static_cast<size_t>(frame_width) * frame_height * channels);
	glGetTextureSubImage(
		sheet->id(), 0,
		x, y, 0,
		frame_width, frame_height, 1,
		format, GL_UNSIGNED_BYTE,
		static_cast<GLsizei>(cell.size()), cell.data()
	);

	// NOTE: i could add the spritesheet's name to the cell's name,
	// but i think this is a unnecessary load on creating a cell
	std::shared_ptr<Texture> tex = std::make_shared<Texture>(cell.data(), frame_width, frame_height, channels);
	this->textures[hash] = { tex, "[CELL (" + std::to_string(x) + ", " + std::to_string(y) + ")]" };
	return tex;
}




std::shared_ptr<Font> Assets::load_font(const char* path, const u32 size) {
	if(path == nullptr) throw std::invalid_argument("Font path is null");
	return this->find_or_create(this->texts, hash::of(path), [&]() {
		return std::make_shared<Font>(path, size);
	});
}

std::shared_ptr<ShaderProgram> Assets::load_program(const char* vertex, const char* fragment) {
	// TODO: format shader if custom goes here
	if(vertex == nullptr && fragment == nullptr) throw std::invalid_argument("Vertex and Fragment Shader source are null");

	// Hash the source content — pointer alone is not stable
	size_t hash = 0;
	if(vertex) hash::combine(hash, std::string_view(vertex));
	if(fragment) hash::combine(hash, std::string_view(fragment));

	return this->find_or_create(this->programs, hash, [&]() {
		const bool separable = ((vertex != nullptr) ^ (fragment != nullptr)); // Decide if must be built to Program Pipeline
		
		std::shared_ptr<ShaderProgram> program = std::make_shared<ShaderProgram>();
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
		return program;
	});
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
	const std::string base_dir = string::base_dir(path) + '/';

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
		path, base_dir.c_str(), true); // path, mtl_basedir, triangulate
	if(!err.empty()) TRACELOG(logger::Warning, "%s", err.c_str());
	if(!ret) throw std::runtime_error(string::format("Failed to load \"%s\": %s", path, err.c_str()));

	// Pre-load all textures referenced by materials
	// Each mat_id maps to its loaded texture (nullptr if none)
	std::vector<std::shared_ptr<Texture>> mat_textures(materials.size(), nullptr);
	for(size_t i = 0; i < materials.size(); i++) {
		const std::string& texname = materials[i].diffuse_texname;

	#if defined(FLOYD_DEBUG_MODEL_LOADING)
		TRACELOG(logger::Debug, "Material [%zu]: diffuse_texname = '%s'", i, texname.c_str());
	#endif

		if(!texname.empty()) {
			const std::string texpath = base_dir + texname;
			mat_textures[i] = this->load_texture(texpath.c_str());

		#if defined(FLOYD_DEBUG_MODEL_LOADING)
			TRACELOG(logger::Debug, "Texture loaded: %s", (mat_textures[i]) ? "OK" : "FAILED");
		#endif
		}
	}

	// Group vertices and indices by material_id across all shapes
	// int key because tinyobj uses -1 for faces with no material
	struct RawGroup {
		std::vector<Vertex> vertices;
		std::vector<u32> indices;
		std::unordered_map<Vertex, u32> uniq_verts; // dedup: hash ->vertex index
	};
	std::vector<RawGroup> material_groups = std::vector<RawGroup>(materials.size() + 1); // +1 for the "-1: no material" slot

	// Dedup only shrinks this, so reserving at this size avoids rehashing and reallocation
	const size_t max_verts = attrib.vertices.size() / 3;
	for(RawGroup& group : material_groups) {
		group.uniq_verts.reserve(max_verts);
		group.vertices.reserve(max_verts);
		group.indices.reserve(max_verts);
	}


	// Each shape = one SubMesh
	for (const tinyobj::shape_t& shape : shapes) {
		size_t index_offset = 0;

		// Iterate per face (robust against n-gons before triangulation)
		for(size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
			const int mat_id = shape.mesh.material_ids[f];
			const int fv = shape.mesh.num_face_vertices[f]; // always 3 since triangulated=true
			RawGroup& group = material_groups[mat_id + 1]; // -1 maps to slot 0

			for(int v = 0; v < fv; v++) {
				const tinyobj::index_t& idx = shape.mesh.indices[index_offset + v];
				Vertex vert{};

				// Position
				vert.pos = {
					attrib.vertices[3 * idx.vertex_index + 0],
					attrib.vertices[3 * idx.vertex_index + 1],
					attrib.vertices[3 * idx.vertex_index + 2]
				};

				// Normal (optional)
				if(idx.normal_index >= 0) {
					vert.normal = {
						attrib.normals[3 * idx.normal_index + 0],
						attrib.normals[3 * idx.normal_index + 1],
						attrib.normals[3 * idx.normal_index + 2]
					};
				}

				// UV
				if(idx.texcoord_index >= 0) {
					vert.uv = {
						attrib.texcoords[2 * idx.texcoord_index + 0],
						attrib.texcoords[2 * idx.texcoord_index + 1] // no y flip, stb_image handles
					};
				}

				// Deduplicate vertices within the material group
				auto [it, inserted] = group.uniq_verts.emplace(vert, (u32)group.vertices.size());
				if(inserted) group.vertices.push_back(vert);
				group.indices.push_back(it->second);
			}
			index_offset += fv;
		}
	}

	VertexLayout layout;
	layout.add<float>(3); // pos
	layout.add<float>(3); // normal
	layout.add<float>(2); // texuv

	// Each material group becomes one independent submesh with its own GPU buffers
	
	std::shared_ptr<Texture> def_tex = this->load<Texture>(hash::of("d_white"));
	for(int mat_id = -1; mat_id < (int)materials.size(); ++mat_id) {
		RawGroup& group = material_groups[mat_id + 1];
		if(group.vertices.empty()) continue; // no faces used this material
	
	#if defined(FLOYD_DEBUG_MODEL_LOADING)
		TRACELOG(logger::Debug, "SubMesh mat_id=%d verts=%zu indices=%zu tex=%s\n",
			mat_id,
			group.vertices.size(),
			group.indices.size(),
			(mat_id >= 0 && mat_textures[mat_id]) ? "SET" : "NULL"
		);
	#endif

		// Load material
		Material mat = Material(this->progs.PROG_VERT_3D, this->progs.PROG_FRAG_3D);
		if(mat_id >= 0 && mat_id < (int)materials.size()) {
			mat.metallic  = materials[mat_id].metallic;
			mat.roughness = materials[mat_id].roughness;
			// Override with object's texture if avaiable
			if(mat_textures[mat_id] != nullptr) mat.albedo = mat_textures[mat_id];
		}

		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(group.vertices, group.indices, layout);
		model->add_submesh(mesh, mat);
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

} // namespace floyd
