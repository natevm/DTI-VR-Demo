#pragma once
#include "gldk.hpp"
#include "Mesh.hpp"
#include "tinyobjloader.h"

#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <cstring>
#include <functional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace Components::Meshes {
	/* An obj mesh contains vertex information from an obj file that has been loaded to the GPU. */
	class OBJMeshGroup : public Mesh {
	public:
		bool generateColors;

		tinyobj::attrib_t attrib;
		
		OBJMeshGroup(std::vector<std::string> filePaths, bool generateColors = false) {
			this->generateColors = generateColors;
			loadFromOBJs(filePaths);
		}

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
		
		/* Loads a mesh from an obj file */
		void loadFromOBJs(std::vector<std::string> objPaths);
		
		GLuint getVertexBuffer() {
			return vertexBuffer;
		}

		GLuint getColorBuffer() {
			return colorBuffer;
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
			return (uint32_t)indices.size();
		}

		void computeCentroid() {
			glm::vec3 s(0.0);
			for (int i = 0; i < points.size(); i += 1) {
				s += points[i];
			}
			s /= points.size();
			centroid = s;
		}

		glm::vec3 getCentroid() {
			return centroid;
		}

		std::vector<glm::vec3> points;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> texcoords;
		std::vector<uint32_t> indices;

	private:
		glm::vec3 centroid = glm::vec3(0.0);

		GLuint vertexBuffer;
		GLuint colorBuffer;
		GLuint indexBuffer;
		GLuint normalBuffer;
		GLuint texCoordBuffer;

		int getIndexBytes() {
			return sizeof(uint32_t);
		}

		void createVertexBuffer() {
			size_t bufferSize = points.size() * sizeof(glm::vec3);
			glCreateBuffers(1, &vertexBuffer);
			glNamedBufferData(vertexBuffer, bufferSize, points.data(), GL_STATIC_DRAW);
		}

		void createColorBuffer() {
			size_t bufferSize = colors.size() * sizeof(glm::vec4);
			glCreateBuffers(1, &colorBuffer);
			glNamedBufferData(colorBuffer, bufferSize, colors.data(), GL_STATIC_DRAW);
		}

		void createIndexBuffer() {
			size_t bufferSize = indices.size() * sizeof(uint32_t);
			glCreateBuffers(1, &indexBuffer);
			glNamedBufferData(indexBuffer, bufferSize, indices.data(), GL_STATIC_DRAW);
		}

		void createNormalBuffer() {
			size_t bufferSize = normals.size() * sizeof(glm::vec3);
			glCreateBuffers(1, &normalBuffer);
			glNamedBufferData(normalBuffer, bufferSize, normals.data(), GL_STATIC_DRAW);
		}

		void createTexCoordBuffer() {
			size_t bufferSize = texcoords.size() * sizeof(glm::vec2);
			glCreateBuffers(1, &texCoordBuffer);
			glNamedBufferData(texCoordBuffer, bufferSize, texcoords.data(), GL_STATIC_DRAW);
		}
	};
}