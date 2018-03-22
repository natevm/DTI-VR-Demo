#pragma once
#include "gldk.hpp"
#include "Texture.hpp"
#include <assert.h>
#include <gli/gli.hpp>
#include <teem/nrrd.h>

namespace Components::Textures {
	class Texture3D : public Texture {

	public:
		/* Constructors */
		Texture3D(std::string imagePath) {
			struct stat st;
			if (stat(imagePath.c_str(), &st) != 0) {
				std::cout<<"Texture3D Error: " + imagePath + " does not exist!"; //(TODO: add placeholder volume)
				imagePath = ResourcePath "Defaults/missing-texture.ktx";
				return;
			}

			if (imagePath.substr(imagePath.find_last_of(".") + 1) != "nrrd") {
				std::cout << "Texture3D Error: Only .nhdr files supported!";
				imagePath = ResourcePath "Defaults/missing-texture.ktx";
			}
			
			createTextureImage(imagePath);
		}

		void cleanup() {

		}

		void createTextureImage(std::string imagePath) {
			char *err;
			Nrrd *nin;

			/* create a nrrd; at this point this is just an empty container */
			nin = nrrdNew();

			/* read in the nrrd from file */
			if (nrrdLoad(nin, imagePath.c_str(), NULL)) {
				err = biffGetDone(NRRD);
				fprintf(stderr, "Texture3D: trouble reading \"%s\":\n%s", imagePath.c_str(), err);
				free(err);
				return;
			}

			/* say something about the array */
			printf("Texture3D: \"%s\" is a %d-dimensional nrrd of type %d (%s)\n",
				imagePath.c_str(), nin->dim, nin->type,
				airEnumStr(nrrdType, nin->type));
			printf("Texture3D: the array contains %d elements, each %d bytes in size\n",
				(int)nrrdElementNumber(nin), (int)nrrdElementSize(nin));

			//nrrdSwapEndian(nin);

			/* Load the texture */
			format = GL_RED;
			if (nin->type == 2)
				internalFormat = GL_R8;
			else if (nin->type == 4)
				internalFormat = GL_R16;
			else if (nin->type == 9)
				internalFormat = GL_R32F;
			else throw std::runtime_error("Texture3D: unsupported volume type " + std::string(airEnumStr(nrrdType, nin->type)));

			GLenum type;
			if (nin->type == 2)
				type = GL_BYTE;
			else if (nin->type == 4)
				type = GL_UNSIGNED_SHORT;
			else if (nin->type == 9)
				type = GL_FLOAT;

			if (nin->dim != 3) throw std::runtime_error("Texture3D: unsupported nrrd dim " + nin->dim);


			width = nin->axis[0].size;
			height = nin->axis[1].size;
			depth = nin->axis[2].size;

			widthSpacing = nin->axis[0].spacing;
			heightSpacing = nin->axis[1].spacing;
			depthSpacing = nin->axis[2].spacing;

			// Copy texture data into buffer
			glCreateTextures(GL_TEXTURE_3D, 1, &textureHandle);
			glTextureStorage3D(textureHandle, 1, internalFormat, width, height, depth);
			glTextureSubImage3D(textureHandle, 0, 0, 0, 0, width, height, depth, format, type, nin->data);
			glTextureParameteri(textureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			/* blow away both the Nrrd struct *and* the memory at nin->data
			(nrrdNix() frees the struct but not the data,
			nrrdEmpty() frees the data but not the struct) */
			nrrdNuke(nin);
		}

		GLuint getColorImageHandle() {
			return textureHandle;
		}

		GLuint getFormat() {
			return format;
		}

		float widthSpacing, heightSpacing, depthSpacing;
		uint32_t width, height, depth, mipLevels;
	private:
		GLuint textureHandle;
		GLuint format, internalFormat;
	};
}