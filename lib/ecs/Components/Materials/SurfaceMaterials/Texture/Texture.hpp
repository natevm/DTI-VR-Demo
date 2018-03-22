#pragma once
#include "Components/Materials/Material.hpp"
#include "Components/Textures/Texture.hpp"
#include "Entities/Entity.hpp"
#include "gldk.hpp"

#include <memory>
#include <glm/glm.hpp>
#include <array>

namespace Components::Materials::SurfaceMaterials {
	class Texture: public Material {
	public:
		static bool initialized;
		static void Initialize() {
			setupGraphicsPipeline();
			setupUniformLocations();
			setupVAO();
			initialized = true;
		}

		static void Destroy() {
			for (Texture ucs : Textures) ucs.cleanup();
		}

		/* Note: one material instance per entity! Cleanup before destroying GLDK stuff */
		Texture(std::string name, PipelineKey pipelineKey, std::shared_ptr<Components::Textures::Texture> albedo) : Material(name) {
			this->pipelineKey = pipelineKey;
			this->albedo = albedo;
			
			/* Allocate and initialize uniform buffer */
			glCreateBuffers(1, &uboHandle);
			uploadUniforms(glm::mat4(1.0));
			//Textures.push_back(*this);
		}

		void cleanup() {
			/* Cleanup data here */
		}

		void uploadUniforms(glm::mat4 model) {
			/* Update uniform buffer */
			UniformBufferObject ubo = {};
			ubo.model = model;
			ubo.pointSize = pointSize;

			glNamedBufferData(uboHandle, sizeof(UniformBufferObject), &ubo, GL_DYNAMIC_DRAW);
		}

		void render(int renderpass, GLuint cameraUBO, std::shared_ptr<Components::Meshes::Mesh> mesh) {
			if (!active) return;

			if (!initialized) {
				std::cout << "Texture: material not initialized, not rendering!" << std::endl;
				return;
			}

			if (!mesh) {
				std::cout << "Texture: mesh is empty, not rendering!" << std::endl;
				return;
			}
			
			if (renderpass != pipelineKey.renderPass) return;

			glUseProgram(ProgramHandle);
			setPipelineState(pipelineKey);

			/* Get mesh data */
			GLuint indexBuffer = mesh->getIndexBuffer();
			GLuint vertexBuffer = mesh->getVertexBuffer();
			GLuint texCoordBuffer = mesh->getTexCoordBuffer();
			GLuint totalIndices = mesh->getTotalIndices();

			/* Bind Vertex Buffers */
			GLuint vertexBuffers[] = { vertexBuffer, texCoordBuffer };
			GLintptr offsets[] = { 0, 0 };
			GLsizei strides[] = { 3 * sizeof(float), 2 * sizeof(float) };
			glBindVertexArray(VAO);
			glBindVertexBuffers(0, 2, vertexBuffers, offsets, strides);

			/* Bind Descriptors */
			glBindBufferBase(GL_UNIFORM_BUFFER, cameraBlockBinding, cameraUBO);
			glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockBinding, uboHandle);
			glBindTextureUnit(textureBinding, albedo->getColorImageHandle());

			//glBindTextureUnit(textureBinding, albedo->getColorImageID());

			/* Bind index buffer */
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			GLenum indexType = (mesh->getIndexBytes() == sizeof(uint16_t)) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

			/* Draw elements */
			glDrawElements(PipelineSettings[pipelineKey].inputAssembly.topology, totalIndices, indexType, (void*)0);
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

		static std::vector<Texture> Textures;

		struct UniformBufferObject {
			glm::mat4 model;
			float pointSize;
		};

		static void setupGraphicsPipeline() {
			/* TODO: When intel supports 4.6, switch to use SPIR-V*/

			/* Read in shader modules */
			auto vertShaderCode = readFile(ResourcePath "Materials/SurfaceMaterials/Texture/shader.vert");
			auto fragShaderCode = readFile(ResourcePath "Materials/SurfaceMaterials/Texture/shader.frag");

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
			GLuint textureLocation = glGetUniformLocation(ProgramHandle, "albedo");
			glGetUniformuiv(ProgramHandle, textureLocation, &textureBinding);
			
		}

		static void setupVAO() {
			glCreateVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			// layout(location = 0) in vec3 position;
			glEnableVertexAttribArray(0);
			glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
			glVertexAttribBinding(0, 0); // bind to first vertex buffer

			// layout(location = 1) in vec2 ;
			glEnableVertexAttribArray(1);
			glVertexAttribFormat(1, 2, GL_FLOAT, false, 0);
			glVertexAttribBinding(1, 1); // bind to second vertex buffer

			glBindVertexArray(0);
		}

		/* Instanced material properties */
		float pointSize = 4.0;
		GLuint uboHandle;
		std::shared_ptr<Components::Textures::Texture> albedo;
		PipelineKey pipelineKey;
	};
}