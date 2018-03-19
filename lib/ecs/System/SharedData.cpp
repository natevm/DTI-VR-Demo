#pragma once

#include "SharedData.hpp"

namespace System::SharedData {
	std::unordered_map<std::string, std::shared_ptr<Entities::Entity>> Scenes;
	std::unordered_map<std::string, std::shared_ptr<Components::Textures::Texture>> TextureList;
	std::unordered_map<std::string, std::shared_ptr<Components::Meshes::Mesh>> MeshList;

	GLuint boundFramebuffer = 0;
}