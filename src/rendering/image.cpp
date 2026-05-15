#include "floydia/rendering/image.hpp"
#include "floydia/helpers/logger.hpp"
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace floyd {

Image::Image(const char* path) noexcept {
	if(path != nullptr) this->data = stbi_load(path, &this->width, &this->height, &this->channels, 0);
	if(this->data == nullptr) {
		logger::log(logger::Error, "Image \"%s\" not found!", path);
		this->width = 2;
		this->height = 2;
		this->channels = 1;
		this->fallback = true;

		this->data = new uint8[sizeof(Image::FALLBACK_IMAGE)];
		std::memcpy(this->data, Image::FALLBACK_IMAGE, sizeof(Image::FALLBACK_IMAGE));
	}
}

Image::Image(const uint8* data, const u32 width, const u32 height, const u8 channels) noexcept
	: width(width), height(height), channels(channels) {
	if(data == nullptr || width <= 0 || height <= 0 || (channels != 1 && channels != 3 && channels != 4)) {
		logger::log(logger::Error, "Raw image data is invalid!");
		this->width = 2;
		this->height = 2;
		this->channels = 1;
		this->fallback = true;

		this->data = new uint8[sizeof(Image::FALLBACK_IMAGE)];
		std::memcpy(this->data, Image::FALLBACK_IMAGE, sizeof(Image::FALLBACK_IMAGE));
		return;
	}

	const size_t bytes = width * height * channels;
	this->data = new uint8[bytes];
	std::memcpy(this->data, data, bytes);
}

Image::Image(const uint8* data, const int size) noexcept {
	if(data != nullptr) this->data = stbi_load_from_memory(data, (int)size, &this->width, &this->height, &this->channels, 0);
	if(this->data == nullptr) {
		logger::log(logger::Error, "Image data is invalid!", path);
		this->width = 2;
		this->height = 2;
		this->channels = 1;
		this->fallback = true;

		this->data = new uint8[sizeof(Image::FALLBACK_IMAGE)];
		std::memcpy(this->data, Image::FALLBACK_IMAGE, sizeof(Image::FALLBACK_IMAGE));
	}
}

Image::~Image() noexcept {
	if(this->data == nullptr) return;
	if(this->fallback) delete[] this->data;
	else stbi_image_free(this->data);
}

void Image::write_to_file(const char* filename) const noexcept {
	stbi_write_png(filename,
		this->width, this->height, this->channels,
		this->data,
		this->width * this->channels
	);
}

void Image::init_stb_image() noexcept { stbi_set_flip_vertically_on_load(true); }

} // namespace floyd
