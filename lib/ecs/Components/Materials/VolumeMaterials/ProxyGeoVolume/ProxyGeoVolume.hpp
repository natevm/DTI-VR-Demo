#pragma once
#include "Components/Materials/Material.hpp"
#include "Components/Textures/Texture.hpp"
#include "Entities/Entity.hpp"
#include "gldk.hpp"

#include <memory>
#include <glm/glm.hpp>
#include <array>

namespace Components::Materials::VolumeMaterials {
	class ProxyGeoVolume: public Material {
	public:
		static bool initialized;
		static void Initialize() {
			setupGraphicsPipeline();
			setupUniformLocations();
			setupVAO();
			initialized = true;
		}

		static void Destroy() {
			for (ProxyGeoVolume ucs : ProxyGeoVolumes) ucs.cleanup();
		}

		/* Note: one material instance per entity! Cleanup before destroying GLDK stuff */
		ProxyGeoVolume(PipelineKey pipelineKey, std::shared_ptr<Components::Textures::Texture> volumeTexture, glm::vec3 size = glm::vec3(1.0), glm::vec3 offset = glm::vec3(0.0)) {
			this->size = size;
			this->offset = offset;
			this->pipelineKey = pipelineKey;
			this->volumeTexture = volumeTexture;
			
			/* Allocate and initialize uniform buffer */
			glCreateBuffers(1, &uboHandle);
			uploadUniforms(glm::mat4(1.0));
			ProxyGeoVolumes.push_back(*this);
		}

		void cleanup() {
			/* Cleanup data here */
		}

		void uploadUniforms(glm::mat4 model) {
			/* Update uniform buffer */
			UniformBufferObject ubo = {};
			ubo.model = model;
			ubo.scaleMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.0/size.x, 1.0/size.y, 1.0/size.z));
			ubo.pointSize = pointSize;

			glNamedBufferData(uboHandle, sizeof(UniformBufferObject), &ubo, GL_DYNAMIC_DRAW);
		}

		void render(int renderpass, GLuint cameraUBO, std::shared_ptr<Components::Meshes::Mesh> mesh) {
			if (!initialized) {
				std::cout << "ProxyGeoVolume: material not initialized, not rendering!" << std::endl;
				return;
			}

			if (!mesh) {
				std::cout << "ProxyGeoVolume: mesh is empty, not rendering!" << std::endl;
				return;
			}
			
			if (renderpass != pipelineKey.renderPass) return;

			glUseProgram(ProgramHandle);

			/* Get mesh data */
			GLuint indexBuffer = mesh->getIndexBuffer();
			GLuint vertexBuffer = mesh->getVertexBuffer();
			GLuint texCoordBuffer = mesh->getTexCoordBuffer();
			GLuint totalIndices = mesh->getTotalIndices();

			/* Bind Vertex Buffers */
			GLuint vertexBuffers[] = { vertexBuffer };
			GLintptr offsets[] = { 0 };
			GLsizei strides[] = { 3 * sizeof(float) };
			glBindVertexArray(VAO);
			glBindVertexBuffers(0, 1, vertexBuffers, offsets, strides);

			/* Bind Descriptors */
			glBindBufferBase(GL_UNIFORM_BUFFER, cameraBlockBinding, cameraUBO);
			glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockBinding, uboHandle);
			glBindTextureUnit(textureBinding, volumeTexture->getColorImageHandle());

			/* Bind index buffer */
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

			setPipelineState(pipelineKey);

			/* Draw elements */
			glDrawElements(PipelineSettings[pipelineKey].inputAssembly.topology, totalIndices, GL_UNSIGNED_SHORT, (void*)0);
			
			glBindVertexArray(0);
		}

		void setPointSize(float size) {
			pointSize = size;
		}

	private:
		/* Static material properties */
		static GLuint ProgramHandle;
		static GLuint cameraBlockBinding;
		static GLuint uniformBlockBinding;
		static GLuint textureBinding;
		static GLuint VAO;

		static std::vector<ProxyGeoVolume> ProxyGeoVolumes;

		struct UniformBufferObject {
			glm::mat4 model;
			glm::mat4 scaleMatrix;
			float pointSize;
		};

		static void setupGraphicsPipeline() {
			/* TODO: When intel supports 4.6, switch to use SPIR-V*/

			/* Read in shader modules */
			auto vertShaderCode = readFile(ResourcePath "Materials/VolumeMaterials/ProxyGeoVolume/shader.vert");
			auto fragShaderCode = readFile(ResourcePath "Materials/VolumeMaterials/ProxyGeoVolume/shader.frag");

			std::vector<GLuint> shaders(2);

			compileShaderFromGLSL(vertShaderCode, GL_VERTEX_SHADER, shaders[0]);
			compileShaderFromGLSL(fragShaderCode, GL_FRAGMENT_SHADER, shaders[1]);
			linkShaders(shaders, ProgramHandle);
		}

		static void setupUniformLocations() {
			/* Camera UBO */
			cameraBlockBinding = glGetUniformBlockIndex(ProgramHandle, "CameraBufferObject");

			/* Per object UBO */
			uniformBlockBinding = glGetUniformBlockIndex(ProgramHandle, "UniformBufferObject");

			/* Albedo texture unit */
			GLuint textureLocation = glGetUniformLocation(ProgramHandle, "volumeTexture");
			glGetUniformuiv(ProgramHandle, textureLocation, &textureBinding);
			
		}

		static void setupVAO() {
			glCreateVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			// layout(location = 0) in vec3 position;
			glEnableVertexAttribArray(0);
			glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
			glVertexAttribBinding(0, 0); // bind to first vertex buffer

			glBindVertexArray(0);
		}

		/* Instanced material properties */
		float pointSize = 4.0;
		glm::vec3 size;
		glm::vec3 offset;
		GLuint uboHandle;
		std::shared_ptr<Components::Textures::Texture> volumeTexture;
		PipelineKey pipelineKey;
	};
}