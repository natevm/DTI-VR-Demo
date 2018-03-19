#pragma once
#include "gldk.hpp"
#include <glm/glm.hpp>

namespace Components::Meshes {
	/* A mesh component contains vertex information that has been loaded to the GPU. */
	class Mesh {
	public:

		class Vertex {
		public:
			glm::vec3 point = glm::vec3(0.0);
			glm::vec4 color = glm::vec4(1, 0, 1, 1);
			glm::vec3 normal = glm::vec3(0.0);
			glm::vec2 texcoord = glm::vec2(0.0);

			bool operator==(const Vertex &other) const
			{
				bool result =
					(point == other.point
						&& color == other.color
						&& normal == other.normal
						&& texcoord == other.texcoord);
				return result;
			}
		};

		virtual void cleanup() = 0;
		virtual int getIndexBytes() { return 1; }
		virtual GLuint getVertexBuffer() = 0;
		virtual GLuint getColorBuffer() { return 0; };
		virtual GLuint getIndexBuffer() = 0;
		virtual GLuint getTexCoordBuffer() = 0;
		virtual GLuint getNormalBuffer() = 0;
		virtual uint32_t getTotalIndices() = 0;
		virtual glm::vec3 getCentroid() = 0;
	};
}
