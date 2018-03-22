#pragma once

#include "Mesh.hpp"

#include <glm/glm.hpp>
#include <array>

namespace Components::Meshes {
	class Cube : public Mesh {
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

		Cube() {
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
			return 36;
		}

		glm::vec3 getCentroid() {
			return glm::vec3(0);
		}

	private:
		std::array<float, 72> verts = {
			1.0, -1.0, -1.0, 1.0, -1.0, -1.0,
			1.0, -1.0, -1.0, 1.0, -1.0, 1.0,
			1.0, -1.0, 1.0, 1.0, -1.0, 1.0,
			-1.0, -1.0, 1.0, -1.0, -1.0, 1.0,
			-1.0, -1.0, 1.0, -1.0, -1.0, -1.0,
			-1.0, -1.0, -1.0, -1.0, -1.0, -1.0,
			1.0, 1.0, -1.0, 1.0, 1.0, -1.0,
			1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
			-1.0, 1.0, 1.0, -1.0, 1.0, 1.0,
			-1.0, 1.0, 1.0, -1.0, 1.0,-1.0,
			-1.0, 1.0,-1.0, -1.0, 1.0,-1.0
		};
		/* TODO: get real normals for this cube */
		std::array<float, 72> normals = {
			1.0, -1.0, -1.0, 1.0, -1.0, -1.0,
			1.0, -1.0, -1.0, 1.0, -1.0, 1.0,
			1.0, -1.0, 1.0, 1.0, -1.0, 1.0,
			-1.0, -1.0, 1.0, -1.0, -1.0, 1.0,
			-1.0, -1.0, 1.0, -1.0, -1.0, -1.0,
			-1.0, -1.0, -1.0, -1.0, -1.0, -1.0,
			1.0, 1.0, -1.0, 1.0, 1.0, -1.0,
			1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
			-1.0, 1.0, 1.0, -1.0, 1.0, 1.0,
			-1.0, 1.0, 1.0, -1.0, 1.0,-1.0,
			-1.0, 1.0,-1.0, -1.0, 1.0,-1.0
		};
		std::array<float, 48> uvs = {
			0.000199999995f, 0.999800026f, 0.000199999995f, 0.666467011f,
			0.666467011f, 0.333133996f, 0.333133996f, 0.999800026f,
			0.333133996f, 0.666467011f, 0.333532989f, 0.666467011f,
			0.333133996f, 0.666867018f, 0.666467011f, 0.666467011f,
			0.999800026f, 0.000199999995f, 0.000199999995f, 0.666867018f,
			0.999800026f, 0.333133996f, 0.666467011f, 0.000199999995f,
			0.000199999995f, 0.333133996f, 0.000199999995f, 0.333532989f,
			0.333532989f, 0.333133996f, 0.333133996f, 0.333133996f,
			0.333133996f, 0.333532989f, 0.333532989f, 0.333532989f,
			0.333133996f, 0.000199999995f, 0.666467011f, 0.333532989f,
			0.666866004f, 0.000199999995f, 0.000199999995f, 0.000199999995f,
			0.666866004f, 0.333133012f, 0.333532989f, 0.000199999995f,
		};
		std::array<uint16_t, 36> indices = {
			0, 3, 6, 0, 6, 9, 12, 21, 18, 12, 18, 15, 1, 13,
			16, 1, 16, 4, 5, 17, 19, 5, 19, 7, 8, 20, 22, 8, 22,
			10, 14, 2, 11, 14, 11, 23,
		};

		GLuint vertexBuffer;
		GLuint indexBuffer;
		GLuint normalBuffer;
		GLuint texCoordBuffer;

		void createVertexBuffer() {
			size_t bufferSize = verts.size() * sizeof(float);
			glCreateBuffers(1, &vertexBuffer);
			glNamedBufferData(vertexBuffer, bufferSize, verts.data(), GL_STATIC_DRAW);
		}

		void createIndexBuffer() {
			size_t bufferSize = indices.size() * sizeof(unsigned short);
			glCreateBuffers(1, &indexBuffer);
			glNamedBufferData(indexBuffer, bufferSize, indices.data(), GL_STATIC_DRAW);
		}

		void createNormalBuffer() {
			size_t bufferSize = normals.size() * sizeof(float);
			glCreateBuffers(1, &normalBuffer);
			glNamedBufferData(normalBuffer, bufferSize, normals.data(), GL_STATIC_DRAW);
		}

		void createTexCoordBuffer() {
			size_t bufferSize = uvs.size() * sizeof(float);
			glCreateBuffers(1, &texCoordBuffer);
			glNamedBufferData(texCoordBuffer, bufferSize, uvs.data(), GL_STATIC_DRAW);
		}
	};
}