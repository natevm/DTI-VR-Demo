#include "gldk.hpp"
#include "Options/Options.h"
#include "System/Workers.hpp"
#include "System/SharedData.hpp"
#include "Entities/Entity.hpp"
#include "Entities/Model/Model.hpp"
#include "Entities/Cameras/SpinTableCamera.hpp"
#include "Components/Materials/Materials.hpp"
#include "Components/Textures/Textures.hpp"
#include "Components/Meshes/Meshes.hpp"

int numStreamLines = 2800; // 916

auto render = []() {
	auto scene = System::SharedData::Scenes["MainScene"];

	auto lastTime = glfwGetTime();
	
	while (!GLDK::ShouldClose()) {
		auto currentTime = glfwGetTime();
		if (currentTime - lastTime > 1.0 / System::Workers::UpdateRate) {
			System::SharedData::Scenes["MainScene"]->uploadUniforms();
			System::SharedData::Scenes["MainScene"]->children["Camera"]->renderScene(scene);
			glfwSwapBuffers(GLDK::DefaultWindow);
			lastTime = currentTime;
		}
		glfwPollEvents();
	}
};

auto update = []() {
	auto lastTime = glfwGetTime();
	while (!GLDK::ShouldClose()) {
		auto currentTime = glfwGetTime();
		if (currentTime - lastTime > 1.0 / System::Workers::UpdateRate) {
			System::SharedData::Scenes["MainScene"]->update();
			lastTime = currentTime;
		}
	}
};

auto raycast = []() {
	while (!GLDK::ShouldClose()) {
	}
};

void SetupComponents() {
	using namespace System::SharedData;
	using namespace Components;
	using namespace Materials;

	PipelineKey key0 = {};
	PipelineKey key1 = {};
	key0.pipelineIdx = 0;
	key1.pipelineIdx = 1;

	PipelineSettings[key0].inputAssembly.topology = GL_LINES;
	PipelineSettings[key0].rasterizer.lineWidth = 4;

	/* Initialize Materials */
	Materials::SurfaceMaterials::Normal::Initialize();
	Materials::SurfaceMaterials::TexCoord::Initialize();
	Materials::SurfaceMaterials::VertexColor::Initialize();
	Materials::SurfaceMaterials::UniformColor::Initialize();
	Materials::SurfaceMaterials::Texture::Initialize();
	Materials::VolumeMaterials::ProxyGeoVolume::Initialize();

	/* Load controller mesh (in desktop, for testing) */
	MeshList["Controller"] = std::make_shared<Components::Meshes::OBJMesh>(ResourcePath "Vive/Controller/vr_controller_vive_1_5.obj");
	TextureList["Controller"] = std::make_shared<Components::Textures::Texture2D>(ResourcePath "Vive/Controller/onepointfive_texture.ktx");

	/* View aligned slices for volume and image */
	MeshList["ViewAlignedSlices"] = std::make_shared<Components::Meshes::ViewAlignedSlices>(256);
	TextureList["Volume"] = std::make_shared<Components::Textures::Texture3D>(ResourcePath "dwi_fa_clean.nrrd");

	/* Load Streamlines */
	std::vector<std::string> paths;
	for (int i = 0; i < numStreamLines; ++i) {
		std::string path = "";
		path += ResourcePath;
		path += "Tractography_fa_high/Tractography.";
		if (i < 9)
			path += "00" + std::to_string(i + 1);
		else if (i < 99)
			path += "0" + std::to_string(i + 1);
		else
			path += std::to_string(i + 1);
		path += ".obj";
		paths.push_back(path);
	}
	MeshList["tractography"] = std::make_shared<Components::Meshes::OBJMeshGroup>(paths, true);
}

int x = 0;

void SetupEntites() {
	auto mainScene = System::SharedData::Scenes["MainScene"];
	Components::Materials::PipelineKey wireframeKey = { 0, 0 };
	Components::Materials::PipelineKey surfaceKey = { 0, 1 };

	/* Camera */
	auto camera = std::make_shared<Entities::Cameras::SpinTableCamera>("Camera", glm::vec3(0.0, 1.0, 1.0), glm::vec3(0.0, 0.0, 0.0));
	camera->setZoomAcceleration(.01f);
	mainScene->addObject(camera);

	/* Add some stream lines */
	auto tracktography = std::make_shared<Entities::Entity>("Tracktography");
	auto streamline = std::make_shared<Entities::Model>("tractography");

	streamline->transform.SetPosition(glm::vec3(-0.015, 0.05, 0.10));
	streamline->transform.SetScale(glm::vec3(-0.9, 1.12, .9) * 1.5f);
	streamline->transform.SetRotation(1.57, glm::vec3(1.0, 0.0, 0.0));

	auto tractographyMaterial = std::make_shared<Components::Materials::SurfaceMaterials::VertexColor>("VertexColor", wireframeKey);
	streamline->addMaterial(tractographyMaterial);
	streamline->setMesh(System::SharedData::MeshList["tractography"]);
	tracktography->addObject(streamline);
	mainScene->addObject(tracktography);

	///* Volume */
	//auto volumeImage = std::dynamic_pointer_cast<Components::Textures::Texture3D>(System::SharedData::TextureList["Volume"]);
	//auto volume = std::make_shared<Entities::Model>("Volume");
	//volume->transform.SetRotation(1.57, glm::vec3(1.0, 0.0, 0.0));

	//volume->setMesh(System::SharedData::MeshList["ViewAlignedSlices"]);
	//auto volumeMaterial = std::make_shared<Components::Materials::VolumeMaterials::ProxyGeoVolume>(surfaceKey, volumeImage, glm::vec3(1.0, 1., -0.7));
	//auto volumeMaterial2 = std::make_shared<Components::Materials::SurfaceMaterials::UniformColor>(wireframeKey);
	//volumeMaterial2->setColor(1.0, 0.0, 0.0, 1.0);
	//volume->addMaterial(volumeMaterial);
	////volume->addMaterial(volumeMaterial2);

	///* Since VBOs are being uploaded, this needs to be called from the render thread */
	//volume->prerenderCallback = [](std::shared_ptr<Entities::Entity> self) {
	//	if (!glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_RIGHT_CONTROL) && !glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_LEFT_CONTROL)) {
	//		auto viewAlignedSlices = std::dynamic_pointer_cast<Components::Meshes::ViewAlignedSlices>(System::SharedData::MeshList["ViewAlignedSlices"]);
	//		auto camera = System::SharedData::Scenes["MainScene"]->children["Camera"];
	//		viewAlignedSlices->update_axis_aligned(self->transform, camera);
	//	}
	//};
	//mainScene->addObject(volume);
}

void Cleanup() {
	using namespace Components;
	using namespace Materials;
	Materials::SurfaceMaterials::Normal::Destroy();
	Materials::SurfaceMaterials::TexCoord::Destroy();
	Materials::SurfaceMaterials::VertexColor::Destroy();
	Materials::SurfaceMaterials::UniformColor::Destroy();
	Materials::SurfaceMaterials::Texture::Destroy();
	Materials::VolumeMaterials::ProxyGeoVolume::Destroy();
}

void Initialize() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	System::SharedData::Scenes["MainScene"] = std::make_shared<Entities::Entity>("MainScene");
}

void Terminate() {
	System::SharedData::Scenes["MainScene"]->cleanup();
}

int main(int argc, char** argv) {
	/* Process arguments */
	Options::ProcessArgs(argc, argv);

	/* Initialize OpenGL */
	GLDK::InitializationParameters GLDKParams = {};
	GLDKParams.windowWidth = 1024;
	GLDKParams.windowHeight = 1024;
	GLDKParams.windowTitle = "DTI Desktop Demo";
	GLDKParams.GLMajorVersion = 4;
	GLDKParams.GLMinorVersion = 5;

	if (GLDK::Initialize(GLDKParams) != true) return -1;

	/* Setup -> start -> cleanup -> terminate the simulation */
	Initialize();
	SetupComponents();
	SetupEntites();
	
	/* Setup Callbacks */
	System::Workers::Callbacks callbacks;
	callbacks.currentThreadCallback = System::Workers::CallbackTypes::Render;
	callbacks.renderCallback = render;
	callbacks.updateCallback = update;
	callbacks.raycastCallback = raycast;

	/* Start/Stop Workers */
	System::Workers::SetupCallbacks(callbacks);
	System::Workers::Start();
	System::Workers::Stop();
	
	/* Cleanup */
	Cleanup();
	Terminate();

	GLDK::Terminate();
}