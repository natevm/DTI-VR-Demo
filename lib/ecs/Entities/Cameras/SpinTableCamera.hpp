#include "Entities/Entity.hpp"
#include "gldk.hpp"

namespace Entities::Cameras {
	/* A perspective gamera*/
	class SpinTableCamera : public Entity {
	public:
		SpinTableCamera(
			std::string name, 
			glm::vec3 initialPos = glm::vec3(1.0), 
			glm::vec3 rotatePoint = glm::vec3(0), 
			glm::vec3 up = glm::vec3(0.0, 1.0, 0.0), 
			bool alternate = false, 
			float fov = glm::radians(90.f)) 
			: Entity(name)
		{
			this->up = up;
			using namespace glm;
			transform.SetPosition(initialPos);
			this->fov = fov;

			initialRot = conjugate(glm::toQuat(lookAt(initialPos, rotatePoint, up)));
			transform.SetRotation(initialRot);

			this->initialPos = initialPos;
			this->initialRot = initialRot;
			this->rotatePoint = rotatePoint;
			
			/* Create perspective transformation */
			this->perspectives[0].projection = glm::perspective(fov, aspectRatio, nearClippingPlane, farClippingPlane);
			this->alternate = alternate;
		}

		void handleArrowKeys() {
			float arrowSpeed = 10;
			if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_UP)) {
				pitchVelocity += arrowSpeed * rotationAcceleration;
			}
			if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_DOWN)) {
				pitchVelocity -= arrowSpeed * rotationAcceleration;
			}
			if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_LEFT)) {
				yawVelocity += arrowSpeed * rotationAcceleration;
			}
			if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_RIGHT)) {
				yawVelocity -= arrowSpeed * rotationAcceleration;
			}
		}

		void handleMouse() {
			if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_RIGHT_CONTROL) || glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_LEFT_CONTROL)) return;
			/* GLFW doesn't give hold info, so we have to handle it ourselves here. */
			/* GLFW also doesn't supply delta cursor position, so we compute it. */

			if (glfwGetMouseButton(GLDK::DefaultWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				if (!mousePrevPressed) {
					glfwGetCursorPos(GLDK::DefaultWindow, &oldXPos, &oldYPos);
					mousePrevPressed = true;
				}
				else {
					glfwGetCursorPos(GLDK::DefaultWindow, &newXPos, &newYPos);
					yawVelocity += (float)-(newXPos - oldXPos) * rotationAcceleration;
					pitchVelocity += (float)-(newYPos - oldYPos) * rotationAcceleration;
					oldXPos = newXPos;
					oldYPos = newYPos;
				}
			}
			else if (glfwGetMouseButton(GLDK::DefaultWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				if (!mousePrevPressed) {
					glfwGetCursorPos(GLDK::DefaultWindow, &oldXPos, &oldYPos);
					mousePrevPressed = true;
				}
				else {
					glfwGetCursorPos(GLDK::DefaultWindow, &newXPos, &newYPos);
					zoomVelocity += (float)(oldYPos - newYPos) * zoomAcceleration * .1f;
					oldXPos = newXPos;
					oldYPos = newYPos;
				}
			}

			if ((glfwGetMouseButton(GLDK::DefaultWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
				&& (glfwGetMouseButton(GLDK::DefaultWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)) {
				mousePrevPressed = false;
			}
		}

		void handleZoom() {
			float arrowSpeed = 1;
			if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_MINUS)) {
				zoomVelocity -= arrowSpeed * zoomAcceleration;
			}
			if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_EQUAL)) {
				zoomVelocity += arrowSpeed * zoomAcceleration;
			}
		}

		void handleReset() {
			if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_R)) {
				transform.SetPosition(initialPos);
				transform.SetRotation(initialRot);
				pitchVelocity = yawVelocity = 0;
				zoomVelocity = 0;
			}
		}

		void update() {
			if ((!alternate && !(glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_LEFT_ALT)))
				|| (alternate && (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_LEFT_ALT))) )
			{
				handleArrowKeys();
				handleMouse();
				handleZoom();
				handleReset();
			
				zoomVelocity -= zoomVelocity * zoomResistance;
				yawVelocity -= yawVelocity * rotateResistance;
				pitchVelocity -= pitchVelocity * rotateResistance;
				
				if (glm::distance(transform.GetPosition() + glm::normalize(rotatePoint - transform.GetPosition()) * zoomVelocity, rotatePoint) > .01f)
					transform.AddPosition(glm::normalize(rotatePoint - transform.GetPosition()) * zoomVelocity);
				else
					zoomVelocity = 0;

				glm::vec3 currentRight = transform.right.load();
				glm::vec3 currentUp = transform.up.load();

				transform.RotateAround(rotatePoint, currentRight, -pitchVelocity);
				transform.RotateAround(rotatePoint, up, yawVelocity);

				aspectRatio = GLDK::CurrentWindowSize[0] / (float)GLDK::CurrentWindowSize[1];
				this->perspectives[0].projection = glm::perspective(fov, aspectRatio, nearClippingPlane, farClippingPlane);
			}
		}

		void setWindowSize(int width, int height) {
			float aspectRatio = width / (float)height;

			assert(aspectRatio > 0);
			if (aspectRatio > 0) {
				this->aspectRatio = aspectRatio;
				this->perspectives[0].projection = glm::perspective(fov, aspectRatio, nearClippingPlane, farClippingPlane);
			}
		}
		void setFieldOfView(float newFOV) {
			fov = newFOV;
		}

		void setZoomAcceleration(float newZoomAcceleration) {
			zoomAcceleration = newZoomAcceleration;
		}

		glm::mat4 getView() {
			return transform.ParentToLocalMatrix();
		}

		glm::mat4 getProjection() {
			return this->perspectives[0].projection;
		}
		
		float getNear() {
			return nearClippingPlane;
		}
		float getFar() {
			return farClippingPlane;
		}
	private: 
		glm::vec3 initialPos;
		glm::quat initialRot;
		glm::vec3 rotatePoint;
		glm::vec3 up;

		float zoomVelocity = 0.0f;
		float zoomAcceleration = .01f;
		float zoomResistance = .1f;

		float yawVelocity = 0.0f;
		float pitchVelocity = 0.0f;
		float rotationAcceleration = 0.03f;
		float rotateResistance = .1f;

		bool mousePrevPressed = false;
		double oldXPos, oldYPos, newXPos, newYPos;
		bool alternate;
		
		float fov = 45.f;
		float aspectRatio = 1.0f;
		float nearClippingPlane = .001f;
		float farClippingPlane = 1000.0f;
	};
}