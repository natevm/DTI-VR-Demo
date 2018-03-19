#pragma once
#include "gldk.hpp"
#include "Components/Meshes/Mesh.hpp"
#include <memory>
#include <fstream>

namespace Entities { class Entity; }

namespace Components::Materials {
	struct PipelineParameters {
		struct InputAssembly {
			GLenum topology = GL_TRIANGLES;
		} inputAssembly;

		struct Rasterizer {
			GLfloat lineWidth = 1.0f;
			//rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			//rasterizer.depthClampEnable = VK_FALSE; // Helpful for shadow maps.
			//rasterizer.rasterizerDiscardEnable = VK_FALSE; // if true, geometry never goes through rasterization, or to frame buffer
			//rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // could be line or point too. those require GPU features to be enabled.
			GLenum polygonMode = GL_FILL;
			//rasterizer.lineWidth = 1.0f; // anything thicker than 1.0 requires wideLines GPU feature
			//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			//rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			//rasterizer.depthBiasEnable = VK_FALSE;
			//rasterizer.depthBiasConstantFactor = 0.0f;
			//rasterizer.depthBiasClamp = 0.0f;
			//rasterizer.depthBiasSlopeFactor = 0.0f;
		} rasterizer;

		struct DepthStencilState {
			GLboolean depthTestEnable = GL_TRUE;
			GLboolean stencilTestEnable = GL_FALSE;
		} depthStencilState;
	};

	struct PipelineKey {
		uint32_t renderPass = 0;
		uint32_t pipelineIdx = 0;

		bool operator==(const PipelineKey &other) const
		{
			bool result =
				(renderPass == other.renderPass
					&& pipelineIdx == other.pipelineIdx);
			return result;
		}
	};
}

inline void hash_combine(std::size_t& seed) { }

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	hash_combine(seed, rest...);
}

template <>
struct std::hash<Components::Materials::PipelineKey>
{
	size_t operator()(const Components::Materials::PipelineKey& k) const
	{
		std::size_t h = 0;
		hash_combine(h, k.renderPass, k.pipelineIdx);
		return h;
	}
};

namespace Components::Materials {
	extern std::unordered_map<PipelineKey, PipelineParameters> PipelineSettings;
	
	/* A material defines a render interface, and cannot be instantiated directly. */
	class Material {
	public:
		virtual void render(int renderpass, GLuint cameraUBO, std::shared_ptr<Components::Meshes::Mesh> mesh) = 0;
		virtual void uploadUniforms(glm::mat4 model) = 0;
		virtual void cleanup() = 0;

		void setPipelineState(PipelineKey key) {
			/* Rasterizer */
			glLineWidth(PipelineSettings[key].rasterizer.lineWidth);
			glPolygonMode(GL_FRONT_AND_BACK, PipelineSettings[key].rasterizer.polygonMode);

			/* Depth Stencil State */
			(PipelineSettings[key].depthStencilState.depthTestEnable) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
			(PipelineSettings[key].depthStencilState.depthTestEnable) ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
		}


	protected:
		/* Helper function for reading in SPIR-V/GLSL files */
		static std::vector<char> readBinaryFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();

			return buffer;
		}

		static std::vector<char> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();

			return buffer;
		}
			
		static void compileShaderFromGLSL(std::vector<char> glslCode, GLenum shaderType, GLuint &shader) {
			shader = glCreateShader(shaderType);

			const GLchar *myshader = glslCode.data();
			const GLchar *shaderversion = "#version 450\n";
			const GLchar *shaderCode[2] = { shaderversion, myshader };

			glShaderSource(shader, 2, shaderCode, 0);

			/* compile shader */
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				// We don't need the shader anymore.
				glDeleteShader(shader);
				printf("%s", infoLog.data());
				throw(std::runtime_error("Error compiling shader!"));
			}
		}
		static void linkShaders(std::vector<GLuint> shaders, GLuint &program) {
			program = glCreateProgram();

			// Attach our shaders to our program
			for (GLuint shader : shaders) {
				glAttachShader(program, shader);
			}

			// Link our program
			glLinkProgram(program);

			// Always detach shaders after a successful link.
			for (GLuint shader : shaders) {
				glDetachShader(program, shader);
			}

			GLint isLinked = 0;
			glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
			if (isLinked == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

				// We don't need the program anymore.
				glDeleteProgram(program);

				// Use the infoLog as you see fit.
				printf("%s", infoLog.data());

				throw(std::runtime_error("Error linking shaders!"));
			}
		}

		///* All material instances have these */
		//std::unordered_map<uint32_t, GLuint> uniformBuffers;
		//std::unordered_map<uint32_t, GLubyte*> uniformBufferMemories;

		//void createUniformBuffers(std::vector<uint32_t> renderPasses, size_t bufferSize) {
		//	for (int i = 0; i < renderPasses.size(); ++i) {
		//		uniformBufferMemories[renderPasses[i]] = (GLubyte *)malloc(bufferSize);

		//		GLuint uboHandle;
		//		glGenBuffers(1, &uboHandle);
		//		uniformBuffers[renderPasses[i]] = uboHandle;
		//	}
		//}
	};
}
