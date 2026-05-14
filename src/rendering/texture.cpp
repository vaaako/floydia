#include <cstring>
#include <floydia/material/texture.hpp>
#include <floydia/helpers/logger.hpp>
#include <floydia/helpers/opengl.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace floyd {

Texture::Texture(const char* path) {
	stbi_set_flip_vertically_on_load(true);

	int channels;
	bool stbi_allocated = true;
	uint8* data = stbi_load(path, &this->_width, &this->_height, &channels, 4);

	if(data == nullptr) {
		logger::log(logger::Error, "Texture \"%s\" not found!", path);
		this->_width = 2;
		this->_height = 2;
		data = new uint8[sizeof(FALLBACK_TEXTURE)];
		std::memcpy(data, FALLBACK_TEXTURE, sizeof(FALLBACK_TEXTURE));
		stbi_allocated = false;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &this->tex);
	glTextureStorage2D(this->tex, 1, opengl::channel_to_format(channels, true), this->_width, this->_height);
	glTextureSubImage2D(this->tex,
		0, 0, 0, // level, xoffset, yoffset
		this->_width, this->_height,
		opengl::channel_to_format(channels, false), GL_UNSIGNED_BYTE, data
	);

	const Texture::Filter filter = (stbi_allocated) ? Texture::Filter::Linear : Texture::Filter::Nearest;
	this->set_filter(filter);
	this->set_filter(Texture::Filter::Repeat);
	glGenerateTextureMipmap(this->tex);

	if(stbi_allocated) stbi_image_free(data);
	else delete[] data;
}

Texture::Texture(uint8* data, const uint32 width, const uint32 height, const uint8 channels)
	: _width(width), _height(height) {

	bool fallback = false;
	if(data == nullptr) {
		logger::log(logger::Error, "Texture data is nullptr");
		this->_width = 2;
		this->_height = 2;
		data = new uint8[sizeof(FALLBACK_TEXTURE)];
		std::memcpy(data, FALLBACK_TEXTURE, sizeof(FALLBACK_TEXTURE));
		fallback = true;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &this->tex);
	glTextureStorage2D(this->tex, 1, opengl::channel_to_format(channels, true), this->_width, this->_height);
	glTextureSubImage2D(this->tex,
		0, 0, 0, // level, xoffset, yoffset
		this->_width, this->_height,
		opengl::channel_to_format(channels, false), GL_UNSIGNED_BYTE, data
	);

	const Texture::Filter filter = (fallback) ? Texture::Filter::Nearest : Texture::Filter::Linear;
	this->set_filter(filter);
	this->set_filter(Texture::Filter::Repeat);
	glGenerateTextureMipmap(this->tex);

	glGenerateTextureMipmap(this->tex);
	if(fallback) delete[] data;
}

Texture::~Texture() noexcept {
	glDeleteTextures(1, &this->tex);
}

void Texture::set_filter(const Texture::Filter filter) const noexcept {
	const GLint ifilter = (GLint)filter;
	if(ifilter >= GL_REPEAT) {
		glTextureParameteri(this->tex, GL_TEXTURE_WRAP_S, ifilter);
		glTextureParameteri(this->tex, GL_TEXTURE_WRAP_T, ifilter);
	} else {
		glTextureParameteri(this->tex, GL_TEXTURE_MAG_FILTER, ifilter);
		glTextureParameteri(this->tex, GL_TEXTURE_MIN_FILTER, ifilter);
	}
}

} // namespace floyd
