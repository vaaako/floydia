#include <floydia/core/assets.hpp>

#include <floydia/gpu/vertexlayout.hpp>
#include <floydia/geometry/vertex.hpp>

namespace floyd {

Assets::Assets(BufferManager& bm) noexcept {
	this->cube_mesh = this->make_cube_mesh();
	this->quad_mesh = this->make_quad_mesh();

	this->program = bm.load_shaderprogram(
		Shaders::DEFAULT_VERTEX, Shaders::DEFAULT_FRAGMENT
	);
	this->program2d = bm.load_shaderprogram(
		Shaders::DEFAULT_VERTEX_2D, Shaders::DEFAULT_FRAGMENT
	);
	this->program_vertex = bm.load_shaderprogram(Shaders::DEFAULT_VERTEX, nullptr);
	this->program_vertex2d = bm.load_shaderprogram(Shaders::DEFAULT_VERTEX_2D, nullptr);
	this->program_fragment = bm.load_shaderprogram(nullptr, Shaders::DEFAULT_FRAGMENT);

	this->default_material = std::make_shared<Material>(
		this->program_vertex,
		this->program_fragment
		// this->program,
		// nullptr
	);
	this->default_material2d = std::make_shared<Material>(
		this->program_vertex2d,
		this->program_fragment
		// this->program2d,
		// nullptr
	);
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
