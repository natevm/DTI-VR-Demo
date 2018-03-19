#include "TexCoord.hpp"

namespace Components::Materials::SurfaceMaterials {
	bool TexCoord::initialized = false;
	GLuint TexCoord::ProgramHandle;
	GLuint TexCoord::cameraBlockBinding;
	GLuint TexCoord::uniformBlockBinding;
	GLuint TexCoord::VAO;
	std::vector<TexCoord> TexCoord::TexCoords;
}