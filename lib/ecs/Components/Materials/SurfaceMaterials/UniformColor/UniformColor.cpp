#include "UniformColor.hpp"

namespace Components::Materials::SurfaceMaterials {
	bool UniformColor::initialized = false;
	GLuint UniformColor::ProgramHandle;
	GLuint UniformColor::cameraBlockBinding;
	GLuint UniformColor::uniformBlockBinding;
	GLuint UniformColor::VAO;
	std::vector<UniformColor> UniformColor::UniformColors;
}