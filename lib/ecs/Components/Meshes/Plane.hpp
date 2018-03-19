#pragma once

#include "Mesh.hpp"

#include <glm/glm.hpp>
#include <array>

namespace Components::Meshes {
	class Plane : public Mesh {
	public:
		void cleanup() {
			/* Destroy index buffer */
			glDeleteBuffers(1, &indexBuffer);

			/* Destroy vertex buffer */
			glDeleteBuffers(1, &vertexBuffer);

			/* Destroy normal buffer */
			glDeleteBuffers(1, &normalBuffer);

			/* Destroy uv buffer */
			glDeleteBuffers(1, &texCoordBuffer);
		}

		Plane() {
			createVertexBuffer();
			createTexCoordBuffer();
			createIndexBuffer();
			createNormalBuffer();
		}
		
		GLuint getVertexBuffer() {
			return vertexBuffer;
		}

		GLuint getIndexBuffer() {
			return indexBuffer;
		}

		GLuint getNormalBuffer() {
			return normalBuffer;
		}

		GLuint getTexCoordBuffer() {
			return texCoordBuffer;
		}

		uint32_t getTotalIndices() {
			return 6;
		}

		glm::vec3 getCentroid() {
			return glm::vec3(0);
		}

	private:
		std::array<float, 12> verts = {
			-1.f,
			-1.f,
			0.f,
			1.f,
			-1.f,
			0.f,
			-1.f,
			1.f,
			0.f,
			1.f,
			1.f,
			0.f
		};
		/* TODO: get real normals for this cube */
		std::array<float, 12> normals = {
			0.f,
			0.f,
			1.f,
			0.f,
			0.f,
			1.f,
			0.f,
			0.f,
			1.f,
			0.f,
			0.f,
			1.f
		};
		std::array<float, 8> uvs = {
			0.f,
			1.f,
			1.f,
			1.f,
			0.f,
			0.f,
			1.f,
			0.f
		};
		std::array<unsigned short, 36> indices = {
			0,
			1,
			3,
			0,
			3,
			2
		};

		GLuint vertexBuffer;
		GLuint indexBuffer;
		GLuint normalBuffer;
		GLuint texCoordBuffer;

		void createVertexBuffer() {
			size_t bufferSize = verts.size() * sizeof(float);
			glGenBuffers(1, &vertexBuffer);
			glNamedBufferData(vertexBuffer, bufferSize, verts.data(), GL_STATIC_DRAW);
		}

		void createIndexBuffer() {
			size_t bufferSize = indices.size() * sizeof(unsigned short);
			glGenBuffers(1, &indexBuffer);
			glNamedBufferData(indexBuffer, bufferSize, indices.data(), GL_STATIC_DRAW);
		}

		void createNormalBuffer() {
			size_t bufferSize = normals.size() * sizeof(float);
			glGenBuffers(1, &normalBuffer);
			glNamedBufferData(normalBuffer, bufferSize, normals.data(), GL_STATIC_DRAW);
		}

		void createTexCoordBuffer() {
			size_t bufferSize = uvs.size() * sizeof(float);
			glGenBuffers(1, &texCoordBuffer);
			glNamedBufferData(texCoordBuffer, bufferSize, uvs.data(), GL_STATIC_DRAW);
		}
	};
}