#include "HTCVive.hpp"
#include "Components/Meshes/OBJMesh.hpp"
#include "Components/Textures/Texture2D.hpp"
#include "Components/Materials/Material.hpp"
#include "System/Input.hpp"
#include <glm/gtx/matrix_decompose.hpp>

namespace Entities::Cameras {
	HTCVive::HTCVive(std::string name, std::shared_ptr<Entity> scene) : Entity(name, true) {
		rightController = std::make_shared<Entities::Model>("Right Controller");
		leftController = std::make_shared<Entities::Model>("Left Controller");
		scene->addObject(rightController);
		scene->addObject(leftController);
		
		/* First, initialize OpenVR*/
		vr::HmdError hmdError;
		m_pHMD = VR_Init(&hmdError, vr::EVRApplicationType::VRApplication_Scene);
		if (hmdError != vr::HmdError::VRInitError_None) {
			std::cout << "Unable to init VR runtime: " << vr::VR_GetVRInitErrorAsEnglishDescription(hmdError) << std::endl;
			this->~HTCVive();
			return;
		}

		/* Get the render model interface */
		m_pRenderModels = (vr::IVRRenderModels *) vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &hmdError);
		if (!m_pRenderModels) {
			std::cout << "Unable to get render model interface " << vr::VR_GetVRInitErrorAsEnglishDescription(hmdError) << std::endl;
			this->~HTCVive();
			return;
		}

		/* Get screenshot interface (used for taking and sharing screenshots) */
		m_pScreenShots = (vr::IVRScreenshots *) vr::VR_GetGenericInterface(vr::IVRScreenshots_Version, &hmdError);
		
		/* Get overlay interface (for showing dashboard/keyboard, couple other things) */
		m_pOverlay = (vr::IVROverlay *) vr::VR_GetGenericInterface(vr::IVROverlay_Version, &hmdError);

		SetupCompanionWindow();
		SetupCameras();
		SetupStereoRenderTargets();
		//SetupRenderModels();

		/* Initialize the compositor. */
		m_pCompositor = vr::VRCompositor();
		if (!m_pCompositor) {
			std::cout << "Compositor initialization failed. See log file for details." << std::endl;
			this->~HTCVive();
			return;
		}

		m_pCompositor->ShowMirrorWindow();
	}

	inline glm::mat4 HTCVive::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
	{
		if (!m_pHMD) return glm::mat4(1.0);
	
		vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(nEye);
		glm::mat4 matrixObj(
			matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
			matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
			matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
			matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);
	
		return glm::inverse(matrixObj);
	}

	void HTCVive::ComposeProjection(float fLeft, float fRight, float fTop, float fBottom, float zNear, float zFar, vr::HmdMatrix44_t *pmProj)
	{
		float idx = 1.0f / (fRight - fLeft);
		float idy = 1.0f / (fBottom - fTop);
		float idz = 1.0f / (zFar - zNear);
		float sx = fRight + fLeft;
		float sy = fBottom + fTop;

		float(*p)[4] = pmProj->m;
		p[0][0] = 2 * idx; p[0][1] = 0;       p[0][2] = sx * idx;    p[0][3] = 0;
		p[1][0] = 0;       p[1][1] = 2 * idy; p[1][2] = sy * idy;    p[1][3] = 0;
		p[2][0] = 0;       p[2][1] = 0;       p[2][2] = -zFar * idz; p[2][3] = -zFar * zNear*idz;
		p[3][0] = 0;       p[3][1] = 0;       p[3][2] = -1.0f;       p[3][3] = 0;
	}

	void HTCVive::ComposeReverseZProjection(float fLeft, float fRight, float fTop, float fBottom, float zNear, vr::HmdMatrix44_t *pmProj)
	{
		float idx = 1.0f / (fRight - fLeft);
		float idy = 1.0f / (fBottom - fTop);
		float sx = fRight + fLeft;
		float sy = fBottom + fTop;
		float(*p)[4] = pmProj->m;
		p[0][0] = 2 * idx; p[0][1] = 0;       p[0][2] = sx * idx;    p[0][3] = 0;
		p[1][0] = 0;       p[1][1] = 2 * idy; p[1][2] = sy * idy;    p[1][3] = 0;
		p[2][0] = 0;       p[2][1] = 0;       p[2][2] = 0;           p[2][3] = zNear;
		p[3][0] = 0;       p[3][1] = 0;       p[3][2] = -1.0f;       p[3][3] = 0;
	}

	inline glm::mat4 HTCVive::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
	{
		if (!m_pHMD) return glm::mat4(1.0);
	
		float pfLeft = 0, pfRight = 0, pfTop = 0, pfBottom = 0;

		vr::HmdMatrix44_t mat;
		m_pHMD->GetProjectionRaw(nEye, &pfLeft, &pfRight, &pfTop, &pfBottom);
		ComposeReverseZProjection(pfLeft, pfRight, pfTop, pfBottom, .05, &mat);

		return glm::mat4(
			mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
			mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
		);
	}

	void HTCVive::SetupCameras() {
			m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
			m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
			m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
			m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Creates a frame buffer. Returns true if the buffer was set up.
	//          Returns false if the setup failed.
	//-----------------------------------------------------------------------------
	bool HTCVive::CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc) {
		int samples = 8;

		glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

		/* Create the depth buffer for this frame buffer */
		glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT32F, nWidth, nHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

		/* Create a RGBA texture which will be rendered to */
		glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, nWidth, nHeight, true);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);
		
		glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

		glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
		glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

		// check FBO status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) return false;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		return true;
	}

	bool HTCVive::SetupStereoRenderTargets() {
		if (!m_pHMD) return false;

		m_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);

		if (!CreateFrameBuffer(m_nRenderWidth, m_nRenderHeight, leftEyeDesc)) std::cout << "Error creating L stereo render target" << std::endl;
		if (!CreateFrameBuffer(m_nRenderWidth, m_nRenderHeight, rightEyeDesc)) std::cout << "Error creating R stereo render target" << std::endl;
		
		leftPerspective = Components::Math::Perspective(leftEyeDesc.m_nRenderFramebufferId);
		rightPerspective = Components::Math::Perspective(rightEyeDesc.m_nRenderFramebufferId);

		leftPerspective.projection = m_mat4ProjectionLeft * m_mat4eyePosLeft;
		rightPerspective.projection = m_mat4ProjectionRight * m_mat4eyePosRight;
		
		return true;
	}

	void HTCVive::SetupCompanionWindow() {
		if (!m_pHMD) return;

		std::vector<VertexDataWindow> vVerts;

		// left eye verts
		vVerts.push_back(VertexDataWindow(glm::vec2(-1, -1), glm::vec2(0, 1)));
		vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(1, 1)));
		vVerts.push_back(VertexDataWindow(glm::vec2(-1, 1), glm::vec2(0, 0)));
		vVerts.push_back(VertexDataWindow(glm:: vec2(0, 1), glm::vec2(1, 0)));

		// right eye verts
		vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(0, 1)));
		vVerts.push_back(VertexDataWindow(glm::vec2(1, -1), glm::vec2(1, 1)));
		vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(0, 0)));
		vVerts.push_back(VertexDataWindow(glm::vec2(1, 1), glm::vec2(1, 0)));

		GLushort vIndices[] = { 0,1,3,   0,3,2,   4,5,6,   4,7,6 };
		m_uiCompanionWindowIndexSize = _countof(vIndices);

		glGenVertexArrays(1, &m_unCompanionWindowVAO);
		glBindVertexArray(m_unCompanionWindowVAO);

		glGenBuffers(1, &m_glCompanionWindowIDVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer);
		glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_glCompanionWindowIDIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, texCoord));

		glBindVertexArray(0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	HTCVive::~HTCVive() {
		vr::VR_Shutdown();
	}

	void HTCVive::update() {
		// Process SteamVR events
		vr::VREvent_t event;
		while (m_pHMD->PollNextEvent(&event, sizeof(event))) {
			switch (event.eventType)
			{
			case vr::VREvent_TrackedDeviceActivated:
			{
				//SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
				std::cout << "Device " << event.trackedDeviceIndex << " attached. Setting up render model." << std::endl;
				if (event.trackedDeviceIndex == 1) leftControllerAttached = true;
				if (event.trackedDeviceIndex == 2) rightControllerAttached = true;
			}
			break;
			case vr::VREvent_TrackedDeviceDeactivated:
			{
				std::cout << "Device " << event.trackedDeviceIndex << " detached." << std::endl;
				if (event.trackedDeviceIndex == 1) leftControllerAttached = false;
				if (event.trackedDeviceIndex == 2) rightControllerAttached = false;
			}
			break;
			case vr::VREvent_TrackedDeviceUpdated:
			{
				std::cout << "Device " << event.trackedDeviceIndex << " updated." << std::endl;
			}
			break;
			}
		}

		// Process SteamVR controller state
		//for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++) {
		//	vr::VRControllerState_t state;
		//	if (m_pHMD->GetControllerState(unDevice, &state, sizeof(state))) {
		//		if (state.ulButtonPressed != 0)
		//			std::cout << "button pressed!" << endl;
		//	}
		//}


		/* Right controller */
		vr::TrackedDeviceIndex_t rightControllerIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
		if (rightControllerIndex != vr::k_unTrackedDeviceIndexInvalid && m_rTrackedDevicePose[rightControllerIndex].bPoseIsValid) {
			if (m_pHMD->GetControllerState(rightControllerIndex, &rightControllerState, sizeof(rightControllerState))) {
				/*if (rightControllerState.ulButtonPressed != 0) {
					System::Input::RightTrigger = 1;
				}
				else {
					System::Input::RightTrigger = 0;
				}*/
			}
		}

		/* Left controller */
		vr::TrackedDeviceIndex_t leftControllerIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
		if (leftControllerIndex != vr::k_unTrackedDeviceIndexInvalid && m_rTrackedDevicePose[leftControllerIndex].bPoseIsValid) {
			if (m_pHMD->GetControllerState(leftControllerIndex, &leftControllerState, sizeof(leftControllerState))) {
				/*if (rightControllerState.ulButtonPressed != 0) {
				System::Input::RightTrigger = 1;
				}
				else {
				System::Input::RightTrigger = 0;
				}*/
			}
		}

		/* Now call the callback */
		updateCallback(shared_from_this());
	}

	void HTCVive::renderScene(std::shared_ptr<Entities::Entity> scene, int renderpass, glm::vec4 clearColor, GLfloat clearDepth, GLint clearStencil) {
		scene->prerender();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER);
			
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepth(0.0f);
		glEnable(GL_MULTISAMPLE);

		// Left Eye
		leftPerspective.updateUBO(getWorldToLocalMatrix());
		glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
		glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		scene->render(renderpass, leftPerspective.getUniformBufferHandle());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_MULTISAMPLE);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId);
		glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glEnable(GL_MULTISAMPLE);

		// Right Eye
		rightPerspective.updateUBO(getWorldToLocalMatrix());
		glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
		glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		scene->render(renderpass, rightPerspective.getUniformBufferHandle());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_MULTISAMPLE);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId);
		glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
		UpdateHMDMatrixPose();
		
		glDepthFunc(GL_LESS);
		glDisable(GL_DEPTH_TEST);

	}

	glm::mat4 HTCVive::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
	{
		glm::mat4 matrixObj(
			matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
			matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
			matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
			matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);

		//glm::mat4 matrixObj(
		//	matPose.m[0][0], matPose.m[0][1], matPose.m[0][2], matPose.m[0][3],
		//	matPose.m[1][0], matPose.m[1][1], matPose.m[1][2], matPose.m[1][3],
		//	matPose.m[2][0], matPose.m[2][1], matPose.m[2][2], matPose.m[2][3],
		//	0.0, 0.0, 0.0, 1.0f
		//);
		return matrixObj;
	}

	glm::vec4 getCameraFromView(const glm::mat4 & a_modelView)
	{
		glm::mat3 rotMat(a_modelView);
		glm::vec3 d(a_modelView[3]);
		glm::vec3 retVec = -d * rotMat;
		return glm::vec4(retVec, 1.0);
	}

	void HTCVive::UpdateHMDMatrixPose() {
		if (!m_pHMD)
			return;

		vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

		m_iValidPoseCount = 0;
		m_strPoseClasses = "";
		for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
		{
			if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
			{
				m_iValidPoseCount++;
				m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
				if (m_rDevClassChar[nDevice] == 0)
				{
					switch (m_pHMD->GetTrackedDeviceClass(nDevice))
					{
					case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
					case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
					case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
					case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
					case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
					default:                                       m_rDevClassChar[nDevice] = '?'; break;
					}
				}
				m_strPoseClasses += m_rDevClassChar[nDevice];
			}
		}

		if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		{
			m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
			transform.localToParentPosition = glm::mat4(m_mat4HMDPose);
			m_mat4HMDPose = glm::inverse(m_mat4HMDPose);
			transform.parentToLocalPosition = m_mat4HMDPose;
			transform.UpdateMatrix();


			//(transform.ParentToLocal(), m_mat4eyePosLeft, m_mat4ProjectionLeft)
			/*transform.right = row(transform.LocalToParentMatrix(), 0);
			transform.up = row(transform.LocalToParentMatrix(), 1);
			transform.forward = row(transform.LocalToParentMatrix(), 2);
			transform.position = row(transform.LocalToParentMatrix(), 3);*/
			//transform.forward = m_mat4eyePosLeft * transform.ParentToLocalMatrix() * glm::vec4(transform.worldForward, 0.0);
			//transform.right = glm::vec3(transform.LocalToParentMatrix() * glm::vec4(transform.worldRight, 0.0));
			//transform.up = glm::vec3(transform.LocalToParentMatrix() * glm::vec4(transform.worldUp, 0.0));
		}
		
		vr::TrackedDeviceIndex_t leftControllerIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
		vr::TrackedDeviceIndex_t rightControllerIndex = m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

		/* Left controller */
		if (leftControllerIndex != vr::k_unTrackedDeviceIndexInvalid && m_rTrackedDevicePose[leftControllerIndex].bPoseIsValid) {
			m_mat4LCPose = m_rmat4DevicePose[leftControllerIndex];
			//leftController->transform.localToParentMatrix = m_mat4LCPose;
			//leftController->transform.parentToLocalMatrix = glm::inverse(m_mat4LCPose);
			glm::vec3 scale, translation, skew;
			glm::vec4 perspective;
			glm::quat rotation;
			glm::decompose(m_mat4LCPose, scale, rotation, translation, skew, perspective);

			leftController->transform.SetPosition(translation);
			leftController->transform.SetRotation(rotation);
			//leftController->transform.SetScale(scale);

			//leftController->transform.SetTransform(m_mat4LCPose);
		}

		/* Right controller */
		if (rightControllerIndex != vr::k_unTrackedDeviceIndexInvalid && m_rTrackedDevicePose[rightControllerIndex].bPoseIsValid) {
			m_mat4RCPose = m_rmat4DevicePose[rightControllerIndex];
			//rightController->transform.localToParentMatrix = m_mat4RCPose;
			//rightController->transform.parentToLocalMatrix = glm::inverse(m_mat4RCPose);
			glm::vec3 scale, translation, skew;
			glm::vec4 perspective;
			glm::quat rotation;
			glm::decompose(m_mat4RCPose, scale, rotation, translation, skew, perspective);

			rightController->transform.SetPosition(translation);
			rightController->transform.SetRotation(rotation);

		}
	}
}
