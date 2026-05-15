#include "floydia/geometry/text.hpp"
#include "floydia/core/core.hpp"
#include "floydia/helpers/string.hpp"
#include "floydia/rendering/image.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <cstring>
#include <vector>
#include <algorithm>

namespace floyd {

struct Text::impl {
	stbtt_fontinfo info; // Does not need free
	std::vector<stbtt_bakedchar> cdata = std::vector<stbtt_bakedchar>(96); // ASCII 32 - 127
};

Text::Text(const char* path, const uint32 size)
	: pimpl(std::make_unique<impl>()) {

	if(path == nullptr) throw std::runtime_error(string::format("Font path is null", path));
	const std::vector<uint8> file_buf = string::read_file_bytes(path);
	if(file_buf.empty()) throw std::runtime_error(string::format("Font \"%s\" failed to load", path));
	if(!stbtt_InitFont(&pimpl->info, file_buf.data(), 0)) throw std::runtime_error(string::format("Font \"%s\" failed to init", path));


#if defined(FLOYD_DEBUG_TEXT)
	// Write to image
#endif
}

// Needed for 'impl'
Text::~Text() noexcept {}

const Text::Glyph* Text::glyph(const uint32 codepoint) const noexcept {
	auto it = this->glyphs.find(codepoint);
	return (it != this->glyphs.end()) ? &it->second : nullptr;
}

float Text::measure(const char* text, const float scale) const noexcept {
	float width = 0;
	const uint8* p = (const uint8*)text;
	while(*p) {
		const uint32 cp = this->utf8_next(p); // Extract unicode codepoint
		const Glyph* g = this->glyph(cp);
		if(g) width += g->advance * scale; // Sum widths
	}
	return width;
}

uint32 Text::utf8_next(const uint8*& p) const noexcept {
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
