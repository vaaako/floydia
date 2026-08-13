#pragma once

#include "floydia/types.hpp"

namespace floyd {

// Wrapper for stb_image
struct Image {
	u8* data = nullptr;
	int width; // 'int' because turns into pointer
	int height;
	int channels;
	bool fallback = false;

	// Load from file
	Image(const char* path) noexcept;
	// Load from raw pixels
	Image(const u8* data, const u32 width, const u32 height, const u8 channels) noexcept;
	// Load from image data
	Image(const u8* data, const int size) noexcept;
	~Image() noexcept;

	Image(const Image& other) noexcept = delete;
	Image& operator=(const Image&) noexcept = delete;
	Image(Image&&) noexcept = default;
	Image& operator=(Image&&) noexcept = default;

	// Image size in bytes
	inline constexpr size_t bytes() const noexcept {
		return static_cast<size_t>(this->width) * static_cast<size_t>(this->height) * static_cast<size_t>(this->channels);
	}

	// Write image to file
	void write_to_file(const char* filename) const noexcept;

	// Initializes stb_image once
	static void init_stb_image() noexcept;

	static constexpr u8 FALLBACK_IMAGE[16] = {
		// Row 0
		0, 0, 0, 255,  255, 0, 255, 255, // Purple, Black
		// Row 1
		255, 0, 255, 255,  0, 0, 0, 255 // Black, Purple
	};
};

} // namespace floyd
