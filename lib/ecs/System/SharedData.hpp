// ┌──────────────────────────────────────────────────────────────────┐
// │ Developer : n8vm                                                 |
// │  Shared Data: Contains global simulation data, like a list of    |
// |    textures, meshes, a scene graph, a list of lights, etc        |
// └──────────────────────────────────────────────────────────────────┘

#include <unordered_map>
#include <memory>
#include "gldk.hpp"

/* Forward Declarations */
namespace Entities { class Entity; }
namespace Components::Textures { class Texture; }
namespace Components::Meshes { class Mesh; }

namespace System::SharedData {
	extern std::unordered_map<std::string, std::shared_ptr<Entities::Entity>> Scenes;
	extern std::unordered_map<std::string, std::shared_ptr<Components::Textures::Texture>> TextureList;
	extern std::unordered_map<std::string, std::shared_ptr<Components::Meshes::Mesh>> MeshList;

	extern GLuint boundFramebuffer;
}