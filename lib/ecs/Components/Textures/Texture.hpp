#pragma once
#include "gldk.hpp"
#include <gli/gli.hpp>

namespace Components::Textures {
	class Texture {
	public:
		virtual void cleanup() = 0;
		virtual GLuint getColorImageHandle() { return 0; };
		virtual GLuint getDepthImageHandle() { return 0; };
		virtual GLuint getStencilImageHandle() { return 0; };

		/* Todo: implement bindless textures https://www.khronos.org/opengl/wiki/Bindless_Texture */
		//virtual VkImageView getImageView() = 0;
		//virtual VkImageView getDepthImageView() { return VK_NULL_HANDLE; };
		//virtual VkImageView getColorImageView() { return VK_NULL_HANDLE; };
		//virtual VkSampler getSampler() = 0;
		//virtual VkImageLayout getLayout() = 0;
		

		virtual uint32_t getWidth() { return 0; };
		virtual uint32_t getHeight() { return 0; };
		virtual uint32_t getDepth() { return 0; };
		
		/*Specifies the number of color components in the texture. */
		//virtual GLuint getInternalFormat() = 0;
		///*Specifies the format of the pixel data on the GPU. */
		virtual GLuint getFormat() = 0;
		///*Specifies the per pixel host type.*/
		//virtual GLuint getType() = 0;
		//virtual GLvoid* getData() = 0;
	};
}

