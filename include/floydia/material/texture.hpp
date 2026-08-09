#pragma once

#include <glad/gl.h>
#include <floydia/types.hpp>

namespace floyd {

class Texture final {
	public:
		// Texture filters.
		// Linear and Repeat are added by default
		enum class Filter : u32 {
			Nearest = GL_NEAREST,
			Linear = GL_LINEAR,

			// NOTE: technically these are wrap

			Repeat = GL_REPEAT,
			Clamp = GL_CLAMP_TO_EDGE,
			Mirrored = GL_MIRRORED_REPEAT
		};

	public:
		Texture(const char* path);
		Texture(u8* data, const u32 width, const u32 height, const u8 channels = 4);
		~Texture() noexcept;
		// No copy
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		// Move ok
		Texture(Texture&&) = default;
		Texture& operator=(Texture&&) = default;

		// Bind to a slot
		inline void bind(const u32 slot) const noexcept { glBindTextureUnit(slot, this->tex); }
		// Set a texture filter
		void set_filter(const Texture::Filter filter) const noexcept;

		inline u32 id() const noexcept { return this->tex; }
		inline u32 width() const noexcept { return this->_width; }
		inline u32 height() const noexcept { return this->_height; }
		inline u8 channels() const noexcept { return this->_channels; }

	private:
		GLuint tex = 0;
		int _width = 0;
		int _height = 0;
		u8 _channels = 0;

	private:
		void bake_texture(const u8* data, const u8 channels);
};

} // namespace floyd
