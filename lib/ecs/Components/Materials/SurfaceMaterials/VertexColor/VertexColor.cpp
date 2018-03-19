#include "Components/Materials/Material.hpp"
#include "VertexColor.hpp"

namespace Components::Materials::SurfaceMaterials {
	bool VertexColor::initialized = false;
	GLuint VertexColor::ProgramHandle;
	GLuint VertexColor::cameraBlockBinding;
	GLuint VertexColor::uniformBlockBinding;
	GLuint VertexColor::VAO;
	std::vector<VertexColor> VertexColor::VertexColors;
}