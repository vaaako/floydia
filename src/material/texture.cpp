#include "floydia/rendering/image.hpp"
#include <cstring>
#include <floydia/material/texture.hpp>
#include <floydia/helpers/logger.hpp>
#include <floydia/helpers/opengl.hpp>

namespace floyd {

Texture::Texture(const char* path) {
	Image img = Image(path);
	this->_width = img.width;
	this->_height = img.height;

	glCreateTextures(GL_TEXTURE_2D, 1, &this->tex);
	glTextureStorage2D(this->tex, 1, opengl::channel_to_format(img.channels, true), this->_width, this->_height);
	glTextureSubImage2D(this->tex,
		0, 0, 0, // level, xoffset, yoffset
		this->_width, this->_height,
		opengl::channel_to_format(img.channels, false), GL_UNSIGNED_BYTE, img.data
	);

	if(img.channels == 1) {
		// Swizzle: Unique R channel, read as alpha on shader
		GLint swizzle[] = { GL_RED, GL_RED, GL_RED, GL_RED };
		glTextureParameteriv(this->tex, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	}

	const Texture::Filter filter = (img.fallback) ? Texture::Filter::Linear : Texture::Filter::Nearest;
	this->set_filter(filter);
	this->set_filter(Texture::Filter::Repeat);
	glGenerateTextureMipmap(this->tex);
}

Texture::Texture(uint8* data, const uint32 width, const uint32 height, const uint8 channels)
	: _width(width), _height(height) {

	bool fallback = false;
	if(data == nullptr || width <= 0 || height <= 0 || (channels != 1 && channels != 3 && channels != 4)) {
		logger::log(logger::Error, "Raw image data is invalid!");
		this->_width = 2;
		this->_height = 2;
		fallback = true;

		data = new uint8[sizeof(Image::FALLBACK_IMAGE)];
		std::memcpy(data, Image::FALLBACK_IMAGE, sizeof(Image::FALLBACK_IMAGE));
		return;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &this->tex);
	glTextureStorage2D(this->tex, 1, opengl::channel_to_format(channels, true), this->_width, this->_height);
	glTextureSubImage2D(this->tex,
		0, 0, 0, // level, xoffset, yoffset
		this->_width, this->_height,
		opengl::channel_to_format(channels, false), GL_UNSIGNED_BYTE, data
	);

	if(channels == 1) {
		// Swizzle: Unique R channel, read as alpha on shader
		GLint swizzle[] = { GL_RED, GL_RED, GL_RED, GL_RED };
		glTextureParameteriv(this->tex, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	}

	const Texture::Filter filter = (fallback) ? Texture::Filter::Nearest : Texture::Filter::Linear;
	this->set_filter(filter);
	this->set_filter(Texture::Filter::Repeat);
	glGenerateTextureMipmap(this->tex);
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
