#include "System.hpp"

namespace System {
	using namespace std;
	
	
	
	std::shared_ptr<Scene> MainScene;
	unordered_map<string, std::shared_ptr<Components::Textures::Texture>> TextureList;
	unordered_map<string, shared_ptr<Components::Meshes::Mesh>> MeshList;
};
