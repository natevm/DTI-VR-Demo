#include "Texture.hpp"

namespace Components::Materials::SurfaceMaterials {
	bool Texture::initialized = false;
	GLuint Texture::ProgramHandle;
	GLuint Texture::cameraBlockBinding;
	GLuint Texture::uniformBlockBinding;
	GLuint Texture::textureBinding;
	GLuint Texture::VAO;
	std::vector<Texture> Texture::Textures;
}