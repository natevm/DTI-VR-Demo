//#pragma once
#include <thread>
#include "vkdk.hpp"
#include <unordered_map>

/* Forward declarations */
class Scene;
namespace Entities { class Entity;}
namespace Components::Textures { class Texture; }
namespace Components::Meshes { class Mesh; }

namespace System {
	
	/* Threads */
	extern std::thread *UpdateThread;
	extern std::thread *EventThread;
	extern std::thread *RenderThread;
	extern std::thread *RaycastThread;
	extern std::atomic<bool> quit;

	extern int UpdateRate;
	extern int FrameRate;

	extern std::shared_ptr<Scene> MainScene;

	extern std::unordered_map<std::string, std::shared_ptr<Components::Textures::Texture>> TextureList;
	extern std::unordered_map<std::string, std::shared_ptr<Components::Meshes::Mesh>> MeshList;

	extern void UpdateLoop();
	extern void RaycastLoop();
	extern void RenderLoop();
	extern void EventLoop();

	/* Call but dont define these */
	extern void Initialize();
	extern void Terminate();

	/* Define these */
	extern void SetupAdditionalRenderPasses();
	extern void SetupComponents();
	extern void SetupEntities();
	extern void Start();
	extern void Cleanup();
};