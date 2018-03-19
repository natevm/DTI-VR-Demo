#include "Normal.hpp"

namespace Components::Materials::SurfaceMaterials {
	GLuint Normal::ProgramHandle;
	GLuint Normal::cameraBlockBinding;
	GLuint Normal::uniformBlockBinding;
	GLuint Normal::VAO;
	std::vector<Normal> Normal::Normals;
}