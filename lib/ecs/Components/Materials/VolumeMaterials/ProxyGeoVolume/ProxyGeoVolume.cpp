#include "ProxyGeoVolume.hpp"

namespace Components::Materials::VolumeMaterials {
	bool ProxyGeoVolume::initialized = false;
	GLuint ProxyGeoVolume::ProgramHandle;
	GLuint ProxyGeoVolume::cameraBlockBinding;
	GLuint ProxyGeoVolume::uniformBlockBinding;
	GLuint ProxyGeoVolume::textureBinding;
	GLuint ProxyGeoVolume::VAO;
	std::vector<ProxyGeoVolume> ProxyGeoVolume::ProxyGeoVolumes;
}