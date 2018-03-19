#pragma once

#include "ImguiVive.hpp"

void doRaycast(Entity* entity, glm::vec4 point, glm::vec4 direction) {
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[0] = System::MouseDown[0];
	io.MouseDown[1] = System::MouseDown[1];

	//HACK: use ray/tri intersection tests
	const glm::vec3 p0(1.0f, 0.0f, 1.0f);
	const glm::vec3 p1(-1.0f, 0.0f, -1.0f);
	const glm::vec3 p2(-1.0f, 0.0f, 1.0f);
	const glm::vec3 p3(1.0f, 0.0f, -1.0f);

	glm::vec3 pt;
	glm::vec2 b;
	float dist;

	bool intersect = glm::intersectRayTriangle(glm::vec3(point), glm::vec3(direction), p0, p2, p1, b, dist);
	if (intersect == false) {
		intersect = glm::intersectRayTriangle(glm::vec3(point), glm::vec3(direction), p0, p3, p1, b, dist);
	}

	if (intersect == true)
	{
		pt = point + dist * direction;
		glfwSetInputMode(GLDK::DefaultWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glm::vec2 guiResolution = glm::vec2(512, 512);// imguiSurface->getGuiResolution();
		io.MousePos = ImVec2(guiResolution.x*(-0.5f*pt.x + 0.5f), guiResolution.y - guiResolution.y * (0.5f*pt.z + 0.5f));

		//glm::vec4 tfPt = glm::vec4(
		//	2.0f * glm::vec2(
		//		(io.MousePos.x - tfWindowMousePos.x) / tfWindowSize.x,   
		//		(io.MousePos.y - tfWindowMousePos.y) / tfWindowSize.y
		//	) - 1.0f , 0.0, 1.0);
		//cout << tfPt.x << " " << tfPt.y << " " << tfPt.z << endl;

		Prefabs::ImguiVive::tfWindow->raycast(glm::vec4(Prefabs::ImguiVive::tfWindowMousePos, 0.0, 1.0), glm::vec4(0.0, 1.0, 0.0, 0.0));

	}
	else {
		glfwSetInputMode(GLDK::DefaultWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

namespace Prefabs {
	std::shared_ptr<Prefabs::TransferFunctionWindow> ImguiVive::tfWindow;
	glm::vec2 ImguiVive::tfWindowMousePos;

	void ImguiVive::setupVRGUIComponents() {
		using namespace Components;
		/* Imgui material */
		Materials::ImguiSurface::Initialize();
		imguiSurface = std::make_shared<Materials::ImguiSurface>(1.0, 1.0, .6);
		imguiSurface->setGuiResolution(512, 512);

		/* Mesh to render imgui on */
		mesh = std::make_shared<Meshes::Plane>();

		/* Frame buffer for drawing onto transfer function texture */
		initializeTFFrameBuffer();
	}

	void ImguiVive::setupVRGUIEntities() {
		tfWindow = std::make_shared<Prefabs::TransferFunctionWindow>();

		/* Use the right hand for ray casting */
		System::Raycaster = rightController;

		imguiPlane = std::make_shared<Entities::Model>();
		imguiPlane->setMesh(mesh);
		imguiPlane->addMaterial(imguiSurface);
		imguiPlane->transform.SetRotation(glm::quat(glm::vec3(-3.14, 0, 0)));
		imguiPlane->transform.AddRotation(glm::quat(glm::vec3(0, 0.0, 3.14)));
		imguiPlane->transform.SetPosition(0.0, 0.01, 0.0);
		imguiPlane->transform.SetScale(.2, .2, .2);
		imguiPlane->raycastCallback = doRaycast;
		leftController->addObject("imgui1", imguiPlane);
	}
}