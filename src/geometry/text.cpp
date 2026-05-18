#include "floydia/geometry/text.hpp"
#include "floydia/core/core.hpp"
#include "floydia/helpers/string.hpp"

#if defined(FLOYD_DEBUG_TEXT)
#include "floydia/rendering/image.hpp"
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <vector>

namespace floyd {

struct Text::impl {
	stbtt_fontinfo info; // Does not need free
};

Text::Text(const char* path, const u32 size)
	: Sprite(), pimpl(std::make_unique<impl>()) {

	if(path == nullptr) throw std::runtime_error(string::format("Font path is null", path));
	const std::vector<u8> file_buf = string::read_file_bytes(path);
	if(file_buf.empty()) throw std::runtime_error(string::format("Font \"%s\" failed to load", path));
	if(!stbtt_InitFont(&pimpl->info, file_buf.data(), 0)) throw std::runtime_error(string::format("Font \"%s\" failed to init", path));

	const float scale = stbtt_ScaleForPixelHeight(&pimpl->info, size);
	this->base_height = size;

	// TODO: review
	int ascent_i, descent_i, linegap_i;
	stbtt_GetFontVMetrics(&pimpl->info, &ascent_i, &descent_i, &linegap_i);
	this->base_ascent = ascent_i * scale;

	std::vector<u8> atlas_pixels = std::vector<u8>(Text::ATLAS_SIZE, 0);
	int pen_x = 1; // Start with 1px border to avoid bleeding
	int pen_y = 1;
	int max_row_height = 0;

	// Compute glyph metrics and pack into atlas
	for(u32 c = Text::CHARSET_BEGIN; c <= Text::CHARSET_END; ++c) {
		int glyph_index = stbtt_FindGlyphIndex(&pimpl->info, c);
		if(glyph_index == 0) continue; // missing glyph
	
		// Get glyph bitmap box (unscaled)
		int x0, y0, x1, y1;
		stbtt_GetGlyphBitmapBox(&pimpl->info, glyph_index, scale, scale, &x0, &y0, &x1, &y1);

		// Advance horizontal
		int advance;
		stbtt_GetGlyphHMetrics(&pimpl->info, glyph_index, &advance, nullptr);
		float advance_f = advance * scale;

		// Start a new row if this glyph doesn't fit horizontally
		int glyph_w = 0;
		int glyph_h = 0;
		u8* bitmap = stbtt_GetGlyphBitmap(&pimpl->info, scale, scale,
			glyph_index, &glyph_w, &glyph_h, nullptr, nullptr);

		// Check if pen_x is beyong bounds
		if((pen_x + glyph_w + 1) > (int)Text::ATLAS_WIDTH) {
			pen_x = 1;
			pen_y += max_row_height + 1;
			max_row_height = 0;
		}

		// Copy into atlas
		if(bitmap) {
			for(int row = 0; row < glyph_h; ++row) {
				for(int col = 0; col < glyph_w; ++col) {
					int dst_idx = (pen_y + row) * Text::ATLAS_WIDTH + (pen_x + col);
					atlas_pixels[dst_idx] = bitmap[row * glyph_w + col];
				}
			}
			stbtt_FreeBitmap(bitmap, nullptr);
		}

		// Store raw glyph data (UVs normalized)
		RawGlyph raw;
		raw.advance = advance_f;
		raw.width = static_cast<float>(glyph_w);
		raw.height = static_cast<float>(glyph_h);
		raw.offset_x = static_cast<float>(x0);
		raw.offset_y = static_cast<float>(y0);
		raw.uv0 = glm::vec2(pen_x / (float)Text::ATLAS_WIDTH, pen_y / (float)Text::ATLAS_HEIGHT);
		raw.uv1 = glm::vec2((pen_x + glyph_w) / (float)Text::ATLAS_WIDTH, (pen_y + glyph_h) / (float)Text::ATLAS_HEIGHT);
		this->glyphs[static_cast<u32>(c)] = raw;

		// Advance pen position
		pen_x += glyph_w + 1; // 1px gap between glyphs
		if(glyph_h > max_row_height) max_row_height = glyph_h;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	this->atlas_texture = assets()
		.load_texture(atlas_pixels.data(), Text::ATLAS_WIDTH, Text::ATLAS_HEIGHT, 1);
	this->atlas_texture->set_filter(Texture::Filter::Nearest);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

#if defined(FLOYD_DEBUG_TEXT)
	Image img = Image(atlas_pixels.data(), Text::ATLAS_WIDTH, Text::ATLAS_HEIGHT, 1);
	img.write_to_file("debug_atlas.png");
#endif
}

// Needed for 'impl'
Text::~Text() noexcept {}

Text::Glyph Text::glyph(const u32 codepoint, const float scale) const noexcept {
	auto it = this->glyphs.find(codepoint);
	if(it == this->glyphs.end()) {
		it = this->glyphs.find(' '); // Try space as fallback
		if(it== this->glyphs.end()) it = this->glyphs.begin();
	}

	const RawGlyph& raw = it->second;
	Glyph scaled;
	scaled.advance = raw.advance * scale;
	scaled.width   = raw.width   * scale;
	scaled.height  = raw.height  * scale;
	scaled.offset_x = raw.offset_x * scale;
	scaled.offset_y = raw.offset_y * scale;
	scaled.uv0 = raw.uv0;
	scaled.uv1 = raw.uv1;
	return scaled;
}

float Text::measure(const char* text, const float scale) const noexcept {
	float width = 0;
	const u8* p = (const u8*)text;
	while(*p) {
		const u32 cp = this->utf8_next(p); // Extract unicode codepoint
		Glyph g = this->glyph(cp, scale);
		width += g.advance * scale; // Sum widths
	}
	return width;
}

u32 Text::utf8_next(const u8*& p) const noexcept {
	uint8_t c = (uint8_t)*p++; // Read first byte and advance caller's pointer
	// ASCII = 0xxxxxxx
	// Single-byte UTF-8 directly matches unicode value
	if(c < 0x80) return c; // 1 byte
	
	// 2 bytes:
	// 110xxxxx 10xxxxxx
	if(c < 0xE0) {
		uint32_t r = (c & 0x1F) << 6; // Keep only xxxx and move them to their final position
		// Read continuation byte (10xxxxxx)
		// keep only xxxxxxand merge into result
		r |= (*p++ & 0x3F);
		return r;
	}

	// 3 bytes:
	// 110xxxxx 10xxxxxx 10xxxxxx
	if(c < 0xF0) {
		uint32_t r = (c & 0x0F) << 12; // Keep xxxx and place them in upper bits
		r |= (*p++ & 0x3F) << 6; // Append middle xxxxxx bits
		r |= (*p++ & 0x3F); // Append lower xxxxxx bits
		return r;
	}

	// 4 bytes:
	// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	uint32_t r = (c & 0x07) << 18; // Keep xxx and place them in uppder bits
	// Append remaining xxxxxx groups
	r |= (*p++ & 0x3F) << 12;
	r |= (*p++ & 0x3F) << 6;
	r |= (*p++ & 0x3F);
	return r;
}

// Example with "á":
// 11000011 10100001
//
// Remove header:
// 11000011 -> 00011
// 10100001 -> 100001
//
// Join:
// 00011 100001
//
// Result:
// 11100001 = 255 (U+00E1)

} // namespace floyd
