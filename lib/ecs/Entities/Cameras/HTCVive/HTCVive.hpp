#pragma once
#include "System/SharedData.hpp"
#include "Entities/Model/Model.hpp"
#include <atomic>
#include "openvr.h"
namespace Entities::Cameras {
	class HTCVive : public Entity {
		struct FramebufferDesc
		{
			GLuint m_nDepthBufferId;
			GLuint m_nRenderTextureId;
			GLuint m_nRenderFramebufferId;
			GLuint m_nResolveTextureId;
			GLuint m_nResolveFramebufferId;
		};

	private: // OpenGL bookkeeping
		int m_iTrackedControllerCount;
		int m_iTrackedControllerCount_Last;
		int m_iValidPoseCount;
		int m_iValidPoseCount_Last;

		std::string m_strPoseClasses;                            // what classes we saw poses for this frame
		char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class

		GLuint m_unLensVAO;
		GLuint m_glIDVertBuffer;
		GLuint m_glIDIndexBuffer;
		unsigned int m_uiIndexSize;

	private:
		vr::IVRSystem *m_pHMD;
		vr::IVRRenderModels *m_pRenderModels;
		vr::IVRScreenshots *m_pScreenShots;
		vr::IVROverlay *m_pOverlay;
		vr::IVRCompositor *m_pCompositor;

		GLuint m_unCompanionWindowVAO;
		GLuint m_glCompanionWindowIDVertBuffer;
		GLuint m_glCompanionWindowIDIndexBuffer;
		unsigned int m_uiCompanionWindowIndexSize;

	public:
		glm::mat4 m_mat4HMDPose;
		glm::mat4 m_mat4LCPose;
		glm::mat4 m_mat4RCPose;

		glm::mat4 m_mat4ProjectionLeft;
		glm::mat4 m_mat4ProjectionRight;
		glm::mat4 m_mat4eyePosLeft;
		glm::mat4 m_mat4eyePosRight;
		Components::Math::Perspective leftPerspective;
		Components::Math::Perspective rightPerspective;

		uint32_t m_nRenderWidth;
		uint32_t m_nRenderHeight;

		FramebufferDesc leftEyeDesc;
		FramebufferDesc rightEyeDesc;

		struct VertexDataWindow
		{
			glm::vec2 position;
			glm::vec2 texCoord;

			VertexDataWindow(const glm::vec2 & pos, const glm::vec2 tex) : position(pos), texCoord(tex) {	}
		};

		vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
		glm::mat4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

		std::shared_ptr<Entities::Model> leftController;
		std::shared_ptr<Entities::Model> rightController;
		bool leftControllerAttached = false;
		bool rightControllerAttached = false;

	public:
		HTCVive(std::string name, std::shared_ptr<Entity> scene);
		void update();
		void renderScene(std::shared_ptr<Entities::Entity> scene, int renderpass = 0, glm::vec4 clearColor = glm::vec4(0.0, 0.0, 0.0, 0.0), GLfloat clearDepth = 1, GLint clearStencil = 0);
		void ComposeProjection(float fLeft, float fRight, float fTop, float fBottom, float zNear, float zFar, vr::HmdMatrix44_t *pmProj);
		void ComposeReverseZProjection(float fLeft, float fRight, float fTop, float fBottom, float zNear, vr::HmdMatrix44_t *pmProj);
		inline glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
		inline glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
		void SetupCameras();
		bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc);
		bool SetupStereoRenderTargets();
		void SetupCompanionWindow();
		void UpdateHMDMatrixPose();
		glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
		~HTCVive();
	};
}