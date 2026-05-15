#pragma once

#include <glad/gl.h>
#include <floydia/types.hpp>

namespace floyd {

class Texture final {
	public:
		// Texture filters.
		// Linear and Repeat are added by default
		enum class Filter : uint32 {
			Nearest = GL_NEAREST,
			Linear = GL_LINEAR,

			Repeat = GL_REPEAT,
			Clamp = GL_CLAMP_TO_EDGE,
			Mirrored = GL_MIRRORED_REPEAT
		};

	public:
		Texture();
		Texture(const char* path);
		Texture(uint8* data, const uint32 width, const uint32 height, const uint8 channels = 4);
		~Texture() noexcept;
		// No copy
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		// Move ok
		Texture(Texture&&) = default;
		Texture& operator=(Texture&&) = default;

		// Bind to a slot
		inline void bind(const uint32 slot) const noexcept { glBindTextureUnit(slot, this->tex); }
		// Set a texture filter
		void set_filter(const Texture::Filter filter) const noexcept;

		void bake_texture(const uint8* data, const uint8 channels);

		inline uint32 id() const noexcept { return this->tex; }
		inline uint32 width() const noexcept { return this->_width; }
		inline uint32 height() const noexcept { return this->_height; }

	private:
		GLuint tex = 0;
		int _width = 0;
		int _height = 0;
};

} // namespace floyd
