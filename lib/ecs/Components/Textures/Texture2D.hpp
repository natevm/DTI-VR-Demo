#pragma once
#include "gldk.hpp"
#include "Texture.hpp"
#include <assert.h>
#include <gli/gli.hpp>

namespace Components::Textures {
	class Texture2D : public Texture {

	public:
		/* Constructors */
		Texture2D(std::string imagePath = ResourcePath "Defaults/missing-texture.ktx") {
			struct stat st;
			if (stat(imagePath.c_str(), &st) != 0) {
				std::cout<<"Texture2D Error: " + imagePath + " does not exist!";
				imagePath = ResourcePath "Defaults/missing-texture.ktx";
				createTextureImageKTX(imagePath);
			}
			else if (imagePath.substr(imagePath.find_last_of(".") + 1) == "ktx") {
				createTextureImageKTX(imagePath);
			}
			else if (imagePath.substr(imagePath.find_last_of(".") + 1) == "png") {
				createTextureImagePNG(imagePath);
			}
			else {
				std::cout << "Texture2D Error: Unsupported image format!";
				imagePath = ResourcePath "Defaults/missing-texture.ktx";
				createTextureImageKTX(imagePath);
			}
			
		}

		void cleanup() {

		}

		void createTextureImageKTX(std::string imagePath) {
			/* Load the texture */
			format = GL_RGBA;
			internalFormat = GL_RGBA8;
			gli::texture2d tex2D(gli::load(imagePath));
			assert(!tex2D.empty());

			width = static_cast<uint32_t>(tex2D[0].extent().x);
			height = static_cast<uint32_t>(tex2D[0].extent().y);
			mipLevels = static_cast<uint32_t>(tex2D.levels());

			// Copy texture data into buffer
			glCreateTextures(GL_TEXTURE_2D, 1, &textureHandle);
			glTextureStorage2D(textureHandle, 1, internalFormat, width, height);
			glTextureSubImage2D(textureHandle, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, tex2D.data());
			glTextureParameteri(textureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		void createTextureImagePNG(std::string imagePath) {
			/* Load the texture */
			format = GL_RGBA;
			internalFormat = GL_RGBA8;
			gli::texture2d tex2D(gli::load(imagePath));
			assert(!tex2D.empty());

			width = static_cast<uint32_t>(tex2D[0].extent().x);
			height = static_cast<uint32_t>(tex2D[0].extent().y);
			mipLevels = static_cast<uint32_t>(tex2D.levels());

			// Copy texture data into buffer
			glCreateTextures(GL_TEXTURE_2D, 1, &textureHandle);
			glTextureStorage2D(textureHandle, 1, internalFormat, width, height);
			glTextureSubImage2D(textureHandle, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, tex2D.data());
			glTextureParameteri(textureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		GLuint getColorImageHandle() {
			return textureHandle;
		}

		GLuint getFormat() {
			return format;
		}

	private:
		GLuint textureHandle;
		GLuint format, internalFormat;
		uint32_t width, height, mipLevels;
	};
}