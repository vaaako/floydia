#include "floydia/rendering/image.hpp"
#include "floydia/material/texture.hpp"
#include "floydia/helpers/opengl.hpp"

namespace floyd {

Texture::Texture(const char* path) {
	Image img = Image(path);
	this->_width = img.width;
	this->_height = img.height;
	this->_channels = img.channels;
	this->bake_texture(img.data, img.channels);

	const Texture::Filter filter = (img.fallback) ? Texture::Filter::Nearest : Texture::Filter::Linear;
	this->set_filter(filter);
	this->set_filter(Texture::Filter::Repeat);
	glGenerateTextureMipmap(this->tex);
}

Texture::Texture(u8* data, const u32 width, const u32 height, const u8 channels)
	: _width(width), _height(height), _channels(channels) {

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

void Texture::bake_texture(const u8* data, const u8 channels) {
	glCreateTextures(GL_TEXTURE_2D, 1, &this->tex);
	glTextureStorage2D(this->tex, 1, opengl::channel_to_format(channels, true), this->_width, this->_height);
	glTextureSubImage2D(this->tex,
		0, 0, 0, // level, xoffset, yoffset
		this->_width, this->_height,
		opengl::channel_to_format(channels, false), GL_UNSIGNED_BYTE, data
	);

	// Swizzle: Unique R channel, read as alpha on shader
	if(channels == 1) {
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
