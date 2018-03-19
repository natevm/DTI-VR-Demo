#pragma once
#include "Components/Materials/Material.hpp"
#include "Entities/Entity.hpp"
#include "gldk.hpp"

#include <memory>
#include <glm/glm.hpp>
#include <array>

namespace Components::Materials::SurfaceMaterials {
	class Normal: public Material {
	public:
		static void Initialize() {
			setupGraphicsPipeline();
			setupUniformLocations();
			setupVAO();
		}

		static void Destroy() {
			for (Normal ucs : Normals) ucs.cleanup();
		}

		/* Note: one material instance per entity! Cleanup before destroying GLDK stuff */
		Normal(PipelineKey pipelineKey) {
			this->pipelineKey = pipelineKey;
			
			/* Allocate and initialize uniform buffer */
			glCreateBuffers(1, &uboHandle);
			uploadUniforms(glm::mat4(1.0));
			Normals.push_back(*this);
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
			if (!mesh) {
				std::cout << "Normal: mesh is empty, not rendering!" << std::endl;
				return;
			}
			
			if (renderpass != pipelineKey.renderPass) return;

			glUseProgram(ProgramHandle);
			setPipelineState(pipelineKey);

			/* Get mesh data */
			GLuint indexBuffer = mesh->getIndexBuffer();
			GLuint vertexBuffer = mesh->getVertexBuffer();
			GLuint normalBuffer = mesh->getNormalBuffer();
			GLuint totalIndices = mesh->getTotalIndices();

			/* Bind Vertex Buffers */
			GLuint vertexBuffers[] = { vertexBuffer, normalBuffer };
			GLintptr offsets[] = { 0, 0 };
			GLsizei strides[] = { 3 * sizeof(float), 3 * sizeof(float) };
			glBindVertexArray(VAO);
			glBindVertexBuffers(0, 2, vertexBuffers, offsets, strides);

			/* Bind Descriptors */
			glBindBufferBase(GL_UNIFORM_BUFFER, cameraBlockBinding, cameraUBO);
			glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockBinding, uboHandle);

			/* Bind index buffer */
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			GLenum indexType = (mesh->getIndexBytes() == sizeof(uint16_t)) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

			/* Draw elements */
			glDrawElements(PipelineSettings[pipelineKey].inputAssembly.topology, totalIndices, indexType, (void*)0);
			glBindVertexArray(0);
		}

		void setColor(float r, float g, float b, float a) {
			color = glm::vec4(r, g, b, a);
		}

		void setPointSize(float size) {
			pointSize = size;
		}

	private:
		/* Static material properties */
		static GLuint ProgramHandle;
		static GLuint cameraBlockBinding;
		static GLuint uniformBlockBinding;
		static GLuint VAO;

		static std::vector<Normal> Normals;

		struct UniformBufferObject {
			glm::mat4 model;
			float pointSize;
		};

		static void setupGraphicsPipeline() {
			/* TODO: When intel supports 4.6, switch to use SPIR-V*/

			/* Read in shader modules */
			auto vertShaderCode = readFile(ResourcePath "Materials/SurfaceMaterials/Normal/shader.vert");
			auto fragShaderCode = readFile(ResourcePath "Materials/SurfaceMaterials/Normal/shader.frag");

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
		}

		static void setupVAO() {
			glCreateVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			// layout(location = 0) in vec3 position;
			glEnableVertexAttribArray(0);
			glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
			glVertexAttribBinding(0, 0); // bind to first vertex buffer

			// layout(location = 1) in vec3 normal;
			glEnableVertexAttribArray(1);
			glVertexAttribFormat(1, 3, GL_FLOAT, false, 0);
			glVertexAttribBinding(1, 1); // bind to second vertex buffer

			glBindVertexArray(0);
		}

		/* Instanced material properties */
		glm::vec4 color = glm::vec4(1.0, 0.0, 1.0, 1.0);
		float pointSize = 4.0;
		GLuint uboHandle;
		PipelineKey pipelineKey;
	};
}