#include "OBJMeshGroup.hpp"

//#ifndef TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_IMPLEMENTATION
//#endif

#include "tinyobjloader.h"


namespace std
{
	inline void hash_combine(std::size_t& seed) { }

	template <typename T, typename... Rest>
	inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		hash_combine(seed, rest...);
	}

	template <>
	struct hash<Components::Meshes::Mesh::Vertex>
	{
		size_t operator()(const Components::Meshes::Mesh::Vertex& k) const
		{
			std::size_t h = 0;
			hash_combine(h, k.point, k.color, k.normal, k.texcoord);
			return h;
		}
	};
}

namespace Components::Meshes {
	void OBJMeshGroup::loadFromOBJs(std::vector<std::string> objPaths) {
		struct stat st;

		int indexOffset = 0;
		for (int fileIdx = 0; fileIdx < objPaths.size(); ++fileIdx) {
			if (stat(objPaths[fileIdx].c_str(), &st) != 0){
				std::cout << objPaths[fileIdx] + " does not exist!" << std::endl;
				objPaths[fileIdx] = ResourcePath "Defaults/missing-model.obj";
			}

			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string err;

			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objPaths[fileIdx].c_str())) {
				throw std::runtime_error(err);
			}

			std::vector<Vertex> vertices;
		
			///* If the mesh has a set of shapes, merge them all into one */
			//if (shapes.size() > 0) {
			//	for (const auto& shape : shapes) {
			//		for (const auto& index : shape.mesh.indices) {
			//			Vertex vertex = Vertex();
			//			vertex.point = {
			//				attrib.vertices[3 * index.vertex_index + 0],
			//				attrib.vertices[3 * index.vertex_index + 1],
			//				attrib.vertices[3 * index.vertex_index + 2]
			//			};
			//			if (attrib.colors.size() != 0) {
			//				vertex.color = {
			//					attrib.colors[3 * index.vertex_index + 0],
			//					attrib.colors[3 * index.vertex_index + 1],
			//					attrib.colors[3 * index.vertex_index + 2],
			//					1.f
			//				};
			//			}
			//			if (attrib.normals.size() != 0) {
			//				vertex.normal = {
			//					attrib.normals[3 * index.normal_index + 0],
			//					attrib.normals[3 * index.normal_index + 1],
			//					attrib.normals[3 * index.normal_index + 2]
			//				};
			//			}
			//			if (attrib.texcoords.size() != 0) {
			//				vertex.texcoord = {
			//					attrib.texcoords[2 * index.texcoord_index + 0],
			//					attrib.texcoords[2 * index.texcoord_index + 1]
			//				};
			//			}
			//			vertices.push_back(vertex);
			//		}
			//	}
			//}

			/* If the obj has no shapes, eg polylines, then try looking for per vertex data */
			/*else */if (shapes.size() == 0) {
				for (int idx = 0; idx < attrib.vertices.size() / 3; ++idx) {
					Vertex v = Vertex();
					v.point = glm::vec3(attrib.vertices[(idx * 3)], attrib.vertices[(idx * 3) + 1], attrib.vertices[(idx * 3) + 2]);
					if (attrib.normals.size() != 0) {
						v.normal = glm::vec3(attrib.normals[(idx * 3)], attrib.normals[(idx * 3) + 1], attrib.normals[(idx * 3) + 2]);
					}
					if (attrib.colors.size() != 0) {
						v.normal = glm::vec3(attrib.colors[(idx * 3)], attrib.colors[(idx * 3) + 1], attrib.colors[(idx * 3) + 2]);
					}
					if (attrib.texcoords.size() != 0) {
						v.texcoord = glm::vec2(attrib.texcoords[(idx * 2)], attrib.texcoords[(idx * 2) + 1]);
					}
					vertices.push_back(v);
				}
			}

			if (generateColors && (vertices.size() != 0)) {
				for (int i = 1; i < vertices.size(); ++i) {

					glm::vec3 gradient = glm::normalize(glm::abs(vertices[i].point - vertices[i - 1].point));
					vertices[i - 1].color = glm::vec4((gradient), 1.0f);
				}
				vertices[vertices.size() - 1].color = vertices[vertices.size() - 2].color;
			}

			/* Eliminate duplicate points */
			std::unordered_map<Vertex, uint32_t> uniqueVertexMap = {};
			std::vector<Vertex> uniqueVertices;
			int previousIdx = (fileIdx == 0) ? 0 : indices[indices.size() - 1] + 1;
			for (int i = 0; i < vertices.size(); ++i) {
				int first = i;
				int second = i + 1;


				/* num points in line + 1 = num indices in line */
				if (i != vertices.size() - 1) {
					indices.push_back(first + previousIdx);
					indices.push_back(second + previousIdx);
				}
				
				Vertex vertex = vertices[i];
				//if (uniqueVertexMap.count(vertex) == 0) {
				//	uniqueVertexMap[vertex] = static_cast<uint32_t>(uniqueVertices.size());
				//}
				uniqueVertices.push_back(vertex);
			}

			
			/* Map vertices to buffers */
			for (int i = 0; i < uniqueVertices.size(); ++i) {
				Vertex v = uniqueVertices[i];
				points.push_back(v.point);
				colors.push_back(v.color);
				normals.push_back(v.normal);
				texcoords.push_back(v.texcoord);
			}

			//indexOffset = indices[indices.size() - 1] + 1;
		}

		/* TODO: Upload data to a vulkan device buffer */
		createVertexBuffer();
		createColorBuffer();
		createIndexBuffer();
		createNormalBuffer();
		createTexCoordBuffer();
	}
}