#pragma once
#include "Entities/Entity.hpp"
#include "Components/Materials/Material.hpp"

namespace Entities {
	class Model : public Entity {
	public:
		Model(std::string name) : Entity(name) {
			
		}

		/* A model has a list of material components, which are used to render it */
		std::unordered_map<std::string, std::shared_ptr<Components::Materials::Material>> materials;
	
		/* A model has a mesh component, which is used by the materials for rendering */
		std::shared_ptr<Components::Meshes::Mesh> mesh;

		/* Models use their materials to render meshes */
		virtual void render(int renderpass, GLuint cameraUBO) {
			for (auto material : materials) {
				material.second->render(renderpass, cameraUBO, mesh);
			}

			Entity::render(renderpass, cameraUBO);
		};

		virtual void uploadUniforms(glm::mat4 model) {
			glm::mat4 new_model = model * transform.LocalToParentMatrix();
			for (auto material : materials) {
				material.second->uploadUniforms(new_model);
			}

			Entity::uploadUniforms(model);
		}

		void cleanup() {
			/*for (auto &mat : materials) {
				mat->cleanup();
			}*/
		}

		void setMesh(std::shared_ptr<Components::Meshes::Mesh> mesh) {
			this->mesh = mesh;
		}

		void addMaterial(std::shared_ptr<Components::Materials::Material> material) {
			materials[material->name] = material;
		}
	};
}