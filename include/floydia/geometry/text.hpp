#pragma once

#include "floydia/geometry/sprite.hpp"
#include "floydia/material/texture.hpp"
#include "floydia/types.hpp"

#include <memory>
#include <unordered_map>

// https://github.com/johnWRS/LearnOpenGLTextRenderingImprovement

namespace floyd {

class Text : protected Sprite {
	public:
		struct Glyph {
			// Horizontal advance in pixels
			float advance;
			float width;
			float height;
			// Offset from cursor
			float offset_x;
			// Offset from cursor
			float offset_y;
			// Top-left UV in atlas
			glm::vec2 uv0;
			/// Bottom-right UV in atlas
			glm::vec2 uv1;
		};

		struct alignas(16) GlyphData {
			glm::vec2 pos;
			glm::vec2 size;
			glm::vec2 uv0;
			glm::vec2 uv1;
			vec4<float> color;
		};

	public:
		Text(const char* path, const u32 size);
		~Text() noexcept;

		inline const Texture* atlas() { return this->atlas_texture.get(); }
		inline float line_height() { return this->base_height; }
		inline float ascent() { return this->base_ascent; }

		// Get stored Glyph from codepoint.
		// Fallback to space character if not found
		Glyph glyph(const u32 codepoint, const float scale) const noexcept;
		// Simplified UTF-8 decoder
		// Remove control bits from UTF-8 (1-4 bytes)
		// https://datatracker.ietf.org/doc/html/rfc3629
		u32 utf8_next(const u8*& p) const noexcept;
		// const u8*&: Advance caller's pointer

		// Bake text to a texture
		// std::shared_ptr<Texture> bake_texture(const std::string& text) const noexcept;

	private:
		// Glyph data without scaling
		struct RawGlyph {
			float advance;
			float width;
			float height;
			float offset_x;
			float offset_y;
			glm::vec2 uv0;
			glm::vec2 uv1;
		};

	private:
		struct impl; // hide external libraries from header
		std::unique_ptr<impl> pimpl;

		std::shared_ptr<Texture> atlas_texture;
		std::unordered_map<u32, RawGlyph> glyphs;
		float base_height = 0.0f;
		float base_ascent = 0.0f;

		// ASCII + Common latin characters
		static constexpr u32 ATLAS_WIDTH  = 512;
		static constexpr u32 ATLAS_HEIGHT = 512;
		static constexpr u32 ATLAS_SIZE = ATLAS_WIDTH * ATLAS_HEIGHT;
		static constexpr u32 CHARSET_BEGIN = 32;
		static constexpr u32 CHARSET_END = 255; // latin-1 supplement
		// static constexpr u32 CHARSET_END   = 383; // Latin Extended-A
		// static constexpr u32 CHARSET_END   = 1279; // Latin Ext-B + Cyrillic

	private:
		// Measure width of a text
		float measure(const char* text, const float scale) const noexcept;
};

} // namespace floyd
