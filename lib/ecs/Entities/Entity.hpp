// ┌──────────────────────────────────────────────────────────────────┐
// │ Developer : n8vm                                                 |
// │  Entity: Most basic object in a scene. An Entity has a transform,|
// |    zero to many child entities, and a perspective which can be   |
// |    used to render the scene.                                     |
// └──────────────────────────────────────────────────────────────────┘

#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <thread>
#include "Components/Math/Transform.hpp"
#include "Components/Math/Perspective.hpp"
#include "gldk.hpp"
#include "System/SharedData.hpp"

namespace Entities {

	/* An entity is an object with a simple transform component, and a list of children. 
			Entities facilitate scene graph traversal, and can be extended to incorporate components like
				meshes, textures, sounds, etc. */
	class Entity : public std::enable_shared_from_this<Entity> {
	public:
		Entity(std::string name, bool createDefaultPerspective = true) {
			this->name = name;
			if (createDefaultPerspective) {
				perspectives.push_back(
					Components::Math::Perspective()
				);
			}
		}

		/* An entity has a name, used as a key within it's parent's children. */
		std::string name;

		/* An entity may know it's parent. */
		Entity *parent = nullptr;

		/* An Entity has a transform component, which moves the entity around */
		Components::Math::Transform transform = Components::Math::Transform();

		/* An Entity has one to many perspectives, which allows the entity to "see" into the scene */
		std::vector<Components::Math::Perspective> perspectives = { };

		/* An Entity can have a list of child objects. By default, these objects are transformed 
		relative to this entity. */
		std::unordered_map<std::string, std::shared_ptr<Entity>> children;
		
		/* If an object isn't active, it and it's children aren't updated/rendered/raycast. */
		bool active = true;
		
		/* Before any render passes occur, allows updates to buffers on render thread */
		std::function<void(std::shared_ptr<Entity>) > prerenderCallback = nullptr;

		virtual void prerender() {
			if (prerenderCallback) prerenderCallback(shared_from_this());
			for (auto i : children) {
				if (i.second.get()->active) {
					i.second->prerender();
				}
			}
		}

		/* Entities by themselves don't render. This is for scene graph traversal */
		virtual void render(int renderpass, GLuint cameraUBO) {
			for (auto i : children) {
				if (i.second.get()->active) {
					i.second->render(renderpass, cameraUBO);
				}
			}
		};

		virtual void renderScene(std::shared_ptr<Entities::Entity> scene, int renderpass = 0, glm::vec4 clearColor = glm::vec4(0.0,0.0,0.0,0.0), GLfloat clearDepth = 1, GLint clearStencil = 0) {
			/* First, call the prerender callbacks, allowing any buffers to be updated on the render thread */
			scene->prerender();
			
			for (int i = 0; i < perspectives.size(); ++i) {
				perspectives[i].updateUBO(transform.ParentToLocalMatrix());
				
				if (perspectives[i].framebufferHandle != System::SharedData::boundFramebuffer) {
					System::SharedData::boundFramebuffer = perspectives[i].framebufferHandle;
					perspectives[i].bindFrameBuffer();
				}
				
				/* Clear this frame buffer */
				glClearNamedFramebufferfv(System::SharedData::boundFramebuffer,
					GL_DEPTH, 0, &clearDepth);
				glClearNamedFramebufferfv(System::SharedData::boundFramebuffer,
					GL_COLOR, 0, &clearColor[0]);
				glClearNamedFramebufferiv(System::SharedData::boundFramebuffer,
					GL_STENCIL, 0, &clearStencil);
				
				scene->render(renderpass, perspectives[i].getUniformBufferHandle());
			}

			if (System::SharedData::boundFramebuffer != 0) {
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}

		std::function<void(std::shared_ptr<Entity>) > updateCallback = nullptr;

		virtual void update() {
			if (updateCallback) updateCallback(shared_from_this());
			for (auto i : children) {
				if (i.second.get()->active) {
					i.second->update();
				}
			}
		}
		
		virtual void uploadUniforms(glm::mat4 model = glm::mat4(1.0)) {
			glm::mat4 new_model = model * transform.LocalToParentMatrix();
			for (auto i : children) {
				if (i.second.get()->active) {
					i.second->uploadUniforms(new_model);
				}
			}
		};
	
		virtual void cleanup() {
			for (auto i : children) {
				i.second->cleanup();
			}
		};
		
		/* Each entity receives a recursive point and direction, such that p and d */
		virtual void raycast(glm::vec4 point, glm::vec4 direction) {
			for (auto i : children) {
				if (i.second.get()->active) {
					/* Transform the ray */
					glm::vec4 newPoint = i.second->transform.ParentToLocalMatrix() * point;
					glm::vec4 newDirection = (i.second->transform.ParentToLocalMatrix() * (point + direction)) - newPoint;
					i.second->raycast(newPoint, newDirection);
				}
			}
		}
		
		void addObject(std::shared_ptr<Entity> object) {
			children[object->name] = object;
			children[object->name]->parent = this;
		}

		void removeObject(std::shared_ptr<Entity> object) {
			children.erase(object->name);
		}

		glm::mat4 getWorldToLocalMatrix() {
			glm::mat4 parentMatrix = glm::mat4(1.0);
			if (parent != nullptr) {
				parentMatrix = parent->getWorldToLocalMatrix();
				return transform.ParentToLocalMatrix() * parentMatrix;
			}
			else return transform.ParentToLocalMatrix();
		}

		glm::mat4 getLocalToWorldMatrix() {
			return glm::inverse(getWorldToLocalMatrix());
		}
	};
}
