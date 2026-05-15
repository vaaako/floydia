#pragma once

#include "floydia/geometry/vertex.hpp"
#include "floydia/material/texture.hpp"
#include "floydia/rendering/mesh.hpp"
#include "floydia/types.hpp"

#include <memory>
#include <unordered_map>

// https://github.com/johnWRS/LearnOpenGLTextRenderingImprovement

namespace floyd {

class Text {
	public:
		struct Glyph {
			// Size of glyph (px)
			vec2<int> size;
			// Offset from baseline to left/top of glyph
			vec2<int> bearing;
			float advance;
			uint32 layer; // texture layer
		};

		struct alignas(16) GlyphData {
			glm::vec2 pos;
			glm::vec2 scale;
			vec4<float> color;
			uint32 glyphindex;
			uint32 _pad[3];
		};

		// Slot fixed size. Largest glyph in charset
		GLuint atlas;
	public:
		Text(const char* path, const uint32 size);
		~Text() noexcept;

		// Get stored Glyph from codepoint.
		// Returns 'nullptr' if 'codepoint' not found
		const Glyph* glyph(const uint32 codepoint) const noexcept;
		// Simplified UTF-8 decoder
		// Remove control bits from UTF-8 (1-4 bytes)
		// https://datatracker.ietf.org/doc/html/rfc3629
		uint32 utf8_next(const uint8*& p) const noexcept;
		// const uint8*&: Advance caller's pointer

		// Bake text to a texture
		// std::shared_ptr<Texture> bake_texture(const std::string& text) const noexcept;

	private:
		struct impl; // hide external libraries from header
		std::unique_ptr<impl> pimpl;

		std::unordered_map<uint32, Glyph> glyphs;

		// ASCII + Common latin characters
		static constexpr int ATLAS_WIDTH  = 512;
		static constexpr int ATLAS_HEIGHT = 512;
		static constexpr uint32 CHARSET_BEGIN = 32; // Space
		static constexpr uint32 CHARSET_END = 255; // latin-1 supplement
		static constexpr uint8 ATLAS_PADDING = 2; // px between glyphs

	private:
		// Measure width of a text
		float measure(const char* text, const float scale) const noexcept;
};

} // namespace floyd
