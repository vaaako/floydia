#include "floydia/rendering/image.hpp"
#include "floydia/material/texture.hpp"
#include "floydia/helpers/opengl.hpp"

namespace floyd {

Texture::Texture(const char* path) {
	Image img = Image(path);
	this->_width = img.width;
	this->_height = img.height;

	this->bake_texture(img.data, img.channels);

	const Texture::Filter filter = (img.fallback) ? Texture::Filter::Linear : Texture::Filter::Nearest;
	this->set_filter(filter);
	this->set_filter(Texture::Filter::Repeat);
	glGenerateTextureMipmap(this->tex);
}

Texture::Texture(uint8* data, const uint32 width, const uint32 height, const uint8 channels)
	: _width(width), _height(height) {

	Image img = Image(data, width, height, channels);
	this->_width = img.width;
	this->_height = img.height;

	this->bake_texture(data, img.channels);

	const Texture::Filter filter = (img.fallback) ? Texture::Filter::Nearest : Texture::Filter::Linear;
	this->set_filter(filter);
	this->set_filter(Texture::Filter::Repeat);
	glGenerateTextureMipmap(this->tex);
}

Texture::~Texture() noexcept {
	glDeleteTextures(1, &this->tex);
}

void Texture::bake_texture(const uint8* data, const uint8 channels) {
	glCreateTextures(GL_TEXTURE_2D, 1, &this->tex);
	glTextureStorage2D(this->tex, 1, opengl::channel_to_format(channels, true), this->_width, this->_height);
	glTextureSubImage2D(this->tex,
		0, 0, 0, // level, xoffset, yoffset
		this->_width, this->_height,
		opengl::channel_to_format(channels, false), GL_UNSIGNED_BYTE, data
	);

	if(channels == 1) {
		// Swizzle: Unique R channel, read as alpha on shader
		GLint swizzle[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
		glTextureParameteriv(this->tex, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	}
}

void Texture::set_filter(const Texture::Filter filter) const noexcept {
	GLuint ifilter = (GLuint)filter;
	if(filter == Texture::Filter::Nearest || filter == Texture::Filter::Linear) {
		glTextureParameteri(this->tex, GL_TEXTURE_MAG_FILTER, ifilter);
		glTextureParameteri(this->tex, GL_TEXTURE_MIN_FILTER, ifilter);
	} else {
		glTextureParameteri(this->tex, GL_TEXTURE_WRAP_S, ifilter);
		glTextureParameteri(this->tex, GL_TEXTURE_WRAP_T, ifilter);
	}
}

} // namespace floyd
