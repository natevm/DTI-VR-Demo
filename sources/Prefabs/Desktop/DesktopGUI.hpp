#pragma once
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/intersect.hpp>

#include <iostream>
#include "cldk.hpp"
#include "gldk.hpp"

#include "Options/Options.h"

#include "System/System.hpp"

#include "Entities/Cameras/OrbitCamera.hpp"
#include "Entities/Model/Model.hpp"

#include "Components/Materials/Material.hpp"
#include "Components/Meshes/Mesh.hpp"
#include "Prefabs/Shared/TransferFunctionWindow.hpp"

#include <thread>
#include <atomic>
#include <deque>
using namespace std;
using namespace glm;

using namespace Entities;
using namespace Components;

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

namespace Prefabs {
	class DesktopGUI : public Model {
	public:
		std::shared_ptr<Materials::ImguiSurface> imguiSurface;
		std::shared_ptr<Components::Textures::Texture> histogramTexture;
		std::shared_ptr<Components::Textures::Texture> tfTexture;
		std::shared_ptr<Prefabs::TransferFunctionWindow> tfWindow;
		glm::vec2 tfWindowMousePos;
		glm::vec2 tfWindowSize;


		bool refreshTransferFunction = true;
		GLuint tfFrameBuffer;
		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		
		DesktopGUI() : Model() {
			histogramTexture = System::TextureList["histogramTexture"];
			tfTexture = System::TextureList["transferFunction"];

			setupDesktopGUIComponents();
			setupDesktopGUIEntities();
		}

		void initializeTFFrameBuffer() {
			/* Save previous frame buffer */
			GLint LastFrameBuffer = 0;
			GLint LastViewport[4];
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &LastFrameBuffer);
			glGetIntegerv(GL_VIEWPORT, LastViewport);

			/* A texture to render to */
			glBindTexture(GL_TEXTURE_2D, tfTexture->getID());
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tfTexture->getWidth(), tfTexture->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			print_gl_error();

			/* A custom frame buffer */
			glGenFramebuffers(1, &tfFrameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, tfFrameBuffer);

			/* Finally, configure the frame buffer */
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tfTexture->getID(), 0);
			glDrawBuffers(1, drawBuffers);
			print_gl_error();

			// Always check that our framebuffer is ok
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR" << std::endl;

			/* Restore previous frame buffer */
			glBindFramebuffer(GL_FRAMEBUFFER, LastFrameBuffer);
			glEnable(GL_DEPTH_TEST);
			glViewport(LastViewport[0], LastViewport[1], LastViewport[2], LastViewport[3]);
			print_gl_error();
		}
		
		void setupDesktopGUIComponents() {
			/* Imgui material */
			Materials::ImguiSurface::Initialize();
			imguiSurface = make_shared<Materials::ImguiSurface>(1.0, 1.0, .6);
			imguiSurface->setGuiResolution(1920, 1440);
			
			/* Mesh to render imgui on */
			mesh = make_shared<Meshes::Plane>();
			
			/* Frame buffer for drawing onto transfer function texture */
			initializeTFFrameBuffer();
		}

		void setupDesktopGUIEntities() {
			tfWindow = make_shared<TransferFunctionWindow>();

			addMaterial(imguiSurface);
			transform.AddRotation(glm::quat(glm::vec3(0.0, 0.0, -3.14)));
			transform.AddRotation(glm::quat(glm::vec3(3.14 / 2.0, 0.0, 0.0)));
		}

		void raycast(glm::vec4 point, glm::vec4 direction) {
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
				glm::vec2 guiResolution = imguiSurface->getGuiResolution();
				io.MousePos = ImVec2(guiResolution.x*(-0.5f*pt.x + 0.5f), guiResolution.y - guiResolution.y * (0.5f*pt.z + 0.5f));
				
				//glm::vec4 tfPt = glm::vec4(
				//	2.0f * glm::vec2(
				//		(io.MousePos.x - tfWindowMousePos.x) / tfWindowSize.x,   
				//		(io.MousePos.y - tfWindowMousePos.y) / tfWindowSize.y
				//	) - 1.0f , 0.0, 1.0);
				//cout << tfPt.x << " " << tfPt.y << " " << tfPt.z << endl;
				
				tfWindow->raycast(vec4(tfWindowMousePos, 0.0, 1.0), glm::vec4(0.0, 1.0, 0.0, 0.0));

			}
			else {
				glfwSetInputMode(GLDK::DefaultWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}

		void prerender(glm::mat4 M, glm::mat4 V, glm::mat4 P) {
			renderTransferFunction(M, V, P);
			renderImgui(M, V, P);
			Entity::prerender(M, V, P);
		}

		void update() {
			tfWindow->update();
		}
		//void updateTFVBO() {
		//	using namespace glm;

		//	tfPointsVBOSize = sizeof(vec3) * tfPoints.size();
		//	tfColorsVBOSize = sizeof(vec4) * tfColors.size();

		//	/* Upload the points */
		//	print_gl_error();
		//	glDeleteBuffers(1, &tfPointsVBO);
		//	glGenBuffers(1, &tfPointsVBO);
		//	glBindBuffer(GL_ARRAY_BUFFER, tfPointsVBO);
		//	print_gl_error();

		//	glBufferData(GL_ARRAY_BUFFER, tfPointsVBOSize, tfPoints.data(), GL_STATIC_DRAW);
		//	print_gl_error();

		//	/* Upload the colors */
		//	glDeleteBuffers(1, &tfColorsVBO);
		//	glGenBuffers(1, &tfColorsVBO);
		//	glBindBuffer(GL_ARRAY_BUFFER, tfColorsVBO);
		//	glBufferData(GL_ARRAY_BUFFER, tfColorsVBOSize, tfColors.data(), GL_STATIC_DRAW);
		//	print_gl_error();

		//	unlitColoredSurface->setAttributeData(tfPointsVBO, tfColorsVBO, tfPoints.size());
		//}

		void renderTransferFunction(glm::mat4 M, glm::mat4 V, glm::mat4 P) {
			///* Upload transfer function geometry if changed */
			//if (refreshTransferFunction) {
			//	updateTFVBO();
			//	refreshTransferFunction = false;
			//}

			/* Save previous frame buffer */
			GLint LastFrameBuffer = 0;
			GLint LastViewport[4];
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &LastFrameBuffer);
			glGetIntegerv(GL_VIEWPORT, LastViewport);

			/* Bind transfer function frame buffer */
			glBindFramebuffer(GL_FRAMEBUFFER, tfFrameBuffer);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			print_gl_error();

			/* Constrain viewport to texture */
			glViewport(0, 0, tfTexture->getWidth(), tfTexture->getHeight());
			print_gl_error();

			/* Render the transfer function window */
			auto tfP = glm::ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
			tfWindow->render(glm::mat4(1.0), glm::mat4(1.0), tfP);

			/* Restore previous frame buffer */
			glBindFramebuffer(GL_FRAMEBUFFER, LastFrameBuffer);
			glEnable(GL_DEPTH_TEST);
			glViewport(LastViewport[0], LastViewport[1], LastViewport[2], LastViewport[3]);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

			print_gl_error();
		}

		void renderImgui(glm::mat4 M, glm::mat4 V, glm::mat4 P) {
			imguiSurface->NewFrame();
			ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

			/* Transfer function section */
			ImGui::Begin("Transfer Function Editor");
			{
				ImVec2 region = ImGui::GetContentRegionAvail();
				ImGuiIO& io = ImGui::GetIO();
				ImTextureID hist_tex_id = (ImTextureID)histogramTexture->getID();
				ImTextureID tf_tex_id = (ImTextureID)tfTexture->getID();
				float my_tex_w = (float)region.x;
				float my_tex_h = (float)region.y;

				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImVec2 pos2 = ImGui::GetCursorPos();
				ImGui::Image(hist_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					float focus_sz = 16.0f;
					float focus_x = io.MousePos.x - pos.x - focus_sz * 0.5f; if (focus_x < 0.0f) focus_x = 0.0f; else if (focus_x > my_tex_w - focus_sz) focus_x = my_tex_w - focus_sz;
					float focus_y = io.MousePos.y - pos.y - focus_sz * 0.5f; if (focus_y < 0.0f) focus_y = 0.0f; else if (focus_y > my_tex_h - focus_sz) focus_y = my_tex_h - focus_sz;
					
					/* Save transform info for ray casting */
					//tfWindowSize = glm::vec2(my_tex_w, my_tex_h);
					tfWindowMousePos = 2.0f * glm::vec2(focus_x / my_tex_w, focus_y / my_tex_h) - 1.0f;
					
					ImGui::Text("Min: (%.2f, %.2f)", focus_x, focus_y);
					ImGui::Text("Max: (%.2f, %.2f)", focus_x + focus_sz, focus_y + focus_sz);
					ImVec2 uv0 = ImVec2((focus_x) / my_tex_w, (focus_y) / my_tex_h);
					ImVec2 uv1 = ImVec2((focus_x + focus_sz) / my_tex_w, (focus_y + focus_sz) / my_tex_h);
					ImGui::Image(hist_tex_id, ImVec2(128, 128), uv0, uv1, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
					ImGui::EndTooltip();
				}
				
				ImGui::SetCursorPos(pos2);
				ImGui::Image(tf_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
			}

			/* Buttons and controls */
			ImGui::End();

			ImGui::ShowDemoWindow();
			imguiSurface->RenderGui();
		}
	};
}
