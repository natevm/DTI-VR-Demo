#pragma once 

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "gldk.hpp"
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Components::Math {
	struct PerspectiveBufferObject {
		glm::mat4 View;
		glm::mat4 Projection;
		glm::mat4 ViewInverse;
		glm::mat4 ProjectionInverse;
		float nearPos;
		float farPos;
	};
	
	class Perspective {
	public:
		glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.0f, .1f, 1000.f) * glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));
		GLuint uboHandle = 0;
		GLuint framebufferHandle = 0;
		GLuint colorTextureHandle = 0;
		GLuint depthTextureHandle = 0;

	public:
		Perspective(GLuint framebufferHandle = 0) {
			glCreateBuffers(1, &uboHandle);
			framebufferHandle = 0;
		}
				
		void bindFrameBuffer() {
			glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle);
		}

		void updateUBO(glm::mat4 view) {
			PerspectiveBufferObject pbo = {};
			pbo.View = view;
			pbo.ViewInverse = glm::inverse(view);

			mat4 proj = projection;
			//proj[1][1] *= -1;
			pbo.Projection = proj;
			pbo.ProjectionInverse = glm::inverse(proj);
			pbo.nearPos = getNear();
			pbo.farPos = getFar();

			glNamedBufferData(uboHandle, sizeof(PerspectiveBufferObject), &pbo, GL_DYNAMIC_DRAW);
		}
		
		GLuint getUniformBufferHandle() {
			return uboHandle;
		}

		glm::mat4 ForwardProjection() {
			return projection;
		}

		float getNear() {
			return .1f;
		}

		float getFar() {
			return 1000.0f;
		}
	};
}