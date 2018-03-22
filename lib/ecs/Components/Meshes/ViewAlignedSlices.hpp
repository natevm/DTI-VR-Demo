#pragma once

#include "Mesh.hpp"
#include "Entities/Entity.hpp"
#include <glm/glm.hpp>
#include <array>
#include <glm/gtx/component_wise.hpp>

namespace Components::Meshes {
	class ViewAlignedSlices : public Mesh {
	public:
		ViewAlignedSlices(int numSlices = 256) {
			this->numSlices = numSlices;
			//createVertexBuffer();
			//createTexCoordBuffer();
			//createIndexBuffer();
			print_gl_error();
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

		int getIndexBytes() {
			return sizeof(uint16_t);
		}
		
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

		glm::vec3 getCentroid() {
			return glm::vec3(0);
		}

		int lastAxisUpdated = -1;
		void update_axis_aligned(std::shared_ptr<Entities::Entity> entity, std::shared_ptr<Entities::Entity> camera) {
			using namespace glm;


			vec3 camfrwd = glm::normalize(glm::vec3(entity->getWorldToLocalMatrix() * vec4(camera->transform.forward.load(), 0.0)));

			glm::vec3 toVolume = camfrwd;// glm::normalize(transform.GetPosition() - pos);
			float absX = glm::abs(toVolume.x);
			float absY = glm::abs(toVolume.y);
			float absZ = glm::abs(toVolume.z);

			glm::vec3 start;
			glm::vec3 end;
			glm::vec3 frwd;
			glm::vec3 right;
			glm::vec3 up;

			if (absX > absY && absX > absZ) {
				if (toVolume.x > 0.0) {
					start = vec3(-1.0, 0.0, 0.0);
					end = vec3(1.0, 0.0, 0.0);
					if (lastAxisUpdated == 0) return;
					lastAxisUpdated = 0;
				}
				else {
					start = vec3(1.0, 0.0, 0.0);
					end = vec3(-1.0, 0.0, 0.0);
					if (lastAxisUpdated == 1) return;
					lastAxisUpdated = 1;
				}
				right = vec3(0.0, 1.0, 0.0);
				up = vec3(0.0, 0.0, 1.0);
			}
			
			else if (absY > absX && absY > absZ) {
				if (toVolume.y > 0.0) {
					start = vec3(0.0, -1.0, 0.0);
					end = vec3(0.0, 1.0, 0.0);
					if (lastAxisUpdated == 2) return;
					lastAxisUpdated = 2;
				}
				else {
					start = vec3(0.0, 1.0, 0.0);
					end = vec3(0.0, -1.0, 0.0);
					if (lastAxisUpdated == 3) return;
					lastAxisUpdated = 3;
				}
				right = vec3(1.0, 0.0, 0.0);
				up = vec3(0.0, 0.0, 1.0);
			}

			else if (absZ > absX && absZ > absY) {
				if (toVolume.z > 0.0) {
					start = vec3(0.0, 0.0, -1.0);
					end = vec3(0.0, 0.0, 1.0);
					if (lastAxisUpdated == 4) return;
					lastAxisUpdated = 4;
				}
				else {
					start = vec3(0.0, 0.0, 1.0);
					end = vec3(0.0, 0.0, -1.0);
					if (lastAxisUpdated == 5) return;
					lastAxisUpdated = 5;
				}
				right = vec3(1.0, 0.0, 0.0);
				up = vec3(0.0, 1.0, 0.0);
			}
			
			frwd = normalize(start - end);

			points.clear();
			uvs.clear();
			colors.clear();
			normals.clear();
			indices.clear();
			
			float dAlpha = 1.0f / numSlices;
			for (int i = 0; i < numSlices; ++i) {
				float alpha = i * dAlpha;


				//float alpha = (i * dAlpha) *(i * dAlpha);
				glm::vec3 sPt = (start * alpha) + (end * (1 - alpha));
				glm::vec3 TL = sPt - right + up;
				glm::vec3 TR = sPt + right + up;
				glm::vec3 BL = sPt - right - up;
				glm::vec3 BR = sPt + right - up;

				points.push_back(TL);
				points.push_back(TR);
				points.push_back(BL);
				points.push_back(BR);

				normals.push_back(-frwd);
				normals.push_back(-frwd);
				normals.push_back(-frwd);
				normals.push_back(-frwd);

				uvs.push_back(glm::vec2(0.0, 0.0));
				uvs.push_back(glm::vec2(1.0, 0.0));
				uvs.push_back(glm::vec2(0.0, 1.0));
				uvs.push_back(glm::vec2(1.0, 1.0));

				colors.push_back(glm::vec4(glm::normalize(TL), dAlpha));
				colors.push_back(glm::vec4(glm::normalize(TR), dAlpha));
				colors.push_back(glm::vec4(glm::normalize(BL), dAlpha));
				colors.push_back(glm::vec4(glm::normalize(BR), dAlpha));

				indices.push_back(4 * i + 0);
				indices.push_back(4 * i + 3);
				indices.push_back(4 * i + 1);
				indices.push_back(4 * i + 0);
				indices.push_back(4 * i + 2);
				indices.push_back(4 * i + 3);
			}

			createVertexBuffer();
			createNormalBuffer();
			createColorBuffer();
			createTexCoordBuffer();
			createIndexBuffer();
		}

		void update(Components::Math::Transform &transform, std::shared_ptr<Entities::Entity> camera, float near = 0.0f, float far = 10.0f, float sliceWeighting = .5f) {
			points.clear();
			uvs.clear();
			colors.clear();
			normals.clear();
			indices.clear();

			glm::vec3 pos = glm::vec3(camera->parent->transform.LocalToParentMatrix() * glm::vec4(camera->transform.position.load(), 1.0f));
			glm::vec3 right = camera->transform.right;
			glm::vec3 frwd = camera->transform.forward;
			glm::vec3 up = camera->transform.up;

			/* Bring camera vectors to model space */
			pos = glm::vec3(transform.ParentToLocalMatrix() * glm::vec4(pos, 1.0));
			right = glm::normalize(glm::vec3(transform.ParentToLocalMatrix() * glm::vec4(right, 0.0))) * 10.0f;
			up = glm::normalize(glm::vec3(transform.ParentToLocalMatrix() * glm::vec4(up, 0.0))) * 10.0f;
			//up = glm::vec3(0.0, 1.0, 0.0) * 10.0f;
			frwd = glm::normalize(glm::vec3(transform.ParentToLocalMatrix() * glm::vec4(frwd, 0.0)));

			/* Might make this permanent. Force position to be center of volume. */
			//pos = glm::vec3(0.0, 0.0, 0.0);
			
			/* Find intersection with a 1 by 1 by 1 axis aligned cube at the world origin. */
			float tmin, tmax;
			intersectBox(glm::vec3(-1., -1., -1.), glm::vec3(1., 1., 1.),
				glm::vec3(pos.x, pos.y, pos.z), glm::vec3(frwd.x, frwd.y, frwd.z), tmin, tmax);

			/* Start and end points for the ray */
			//glm::vec3 start = pos + (tmin * frwd);
			glm::vec3 start = pos + (near * frwd);
			//glm::vec3 end = pos + (tmax * frwd);
			glm::vec3 end = pos + (far * frwd);

			float dAlpha = /*(sqrt(2.0))*/1.0f / numSlices;
			for (int i = 0; i < numSlices; ++i) {
				float alpha = pow(i * dAlpha, sliceWeighting);
				//float alpha = (i * dAlpha) *(i * dAlpha);
				glm::vec3 sPt = (start * alpha) + (end * (1 - alpha));
				glm::vec3 TL = sPt - right + up;
				glm::vec3 TR = sPt + right + up;
				glm::vec3 BL = sPt - right - up;
				glm::vec3 BR = sPt + right - up;

				points.push_back(TL);
				points.push_back(TR);
				points.push_back(BL);
				points.push_back(BR);

				normals.push_back(-frwd);
				normals.push_back(-frwd);
				normals.push_back(-frwd);
				normals.push_back(-frwd);

				uvs.push_back(glm::vec2(0.0, 0.0));
				uvs.push_back(glm::vec2(1.0, 0.0));
				uvs.push_back(glm::vec2(0.0, 1.0));
				uvs.push_back(glm::vec2(1.0, 1.0));

				colors.push_back(glm::vec4(glm::normalize(TL), dAlpha));
				colors.push_back(glm::vec4(glm::normalize(TR), dAlpha));
				colors.push_back(glm::vec4(glm::normalize(BL), dAlpha));
				colors.push_back(glm::vec4(glm::normalize(BR), dAlpha));

				indices.push_back(4 * i + 0);
				indices.push_back(4 * i + 3);
				indices.push_back(4 * i + 1);
				indices.push_back(4 * i + 0);
				indices.push_back(4 * i + 2);
				indices.push_back(4 * i + 3);
			}

			createVertexBuffer();
			createNormalBuffer();
			createColorBuffer();
			createTexCoordBuffer();
			createIndexBuffer();
		}

	private:
		int numSlices;

		std::vector<glm::vec3> points;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> uvs;
		std::vector<unsigned short> indices;

		inline void intersectBox(glm::vec3 boxmin, glm::vec3 boxmax, glm::vec3 p,
			glm::vec3 dir, float &tmin, float &tmax) {
			#define MIN(a,b) ((a) < (b) ? (a) : (b))
			#define MAX(a,b) ((a) > (b) ? (a) : (b))
			glm::vec3 dirfrac;
			if (dir.x == 0.0f) dir.x += .0000001f;
			if (dir.y == 0.0f) dir.y += .0000001f;
			if (dir.z == 0.0f) dir.z += .0000001f;

			dirfrac.x = 1.0f / dir.x;
			dirfrac.y = 1.0f / dir.y;
			dirfrac.z = 1.0f / dir.z;

			float t1 = (boxmin.x - p.x) * dirfrac.x;
			float t2 = (boxmax.x - p.x) * dirfrac.x;
			float t3 = (boxmin.y - p.y) * dirfrac.y;
			float t4 = (boxmax.y - p.y) * dirfrac.y;
			float t5 = (boxmin.z - p.z) * dirfrac.z;
			float t6 = (boxmax.z - p.z) * dirfrac.z;

			tmin = MAX(MAX(MIN(t1, t2), MIN(t3, t4)), MIN(t5, t6));
			tmax = MIN(MIN(MAX(t1, t2), MAX(t3, t4)), MAX(t5, t6));
			//if (tmax < 0) // AABB is behind the ray
			//	return false;

			//if (tmin > tmax) // The ray misses the box
			//	return false;

			//if (tmin > hInfo.z)  // The box is behind something closer
			//	return false;
			#undef MIN
			#undef MAX
		}

		//// n: plane normal, p0 : point on plane, l0: ray origin, l: ray direction, t : output
		//inline bool intersectPlane(
		//	const glm::vec3 &n, const glm::vec3 &p0, const glm::vec3 &l0, const glm::vec3 &l,
		//	float tmin, float tmax, float &t)
		//{
		//	// Ray Plane Intersection
		//	//    (p0 - l0) . n
		//	//t = ------------- 
		//	//        l . n
		//	// parallel when l.n close to zero


		//	// assuming vectors are all normalized
		//	float denom = dot(n, l);
		//	if (fabs(denom) > 1e-6) {
		//		glm::vec3 p0l0 = p0 - l0;
		//		t = dot(p0l0, n) / denom;
		//		if (t >= tmin && t <= tmax)
		//			return true;
		//	}

		//	return false;
		//}

		///*! Returns the pseudoangle between the line p1 to (infinity, p1.y) and the
		//line from p1 to p2. The pseudoangle has the property that the ordering of
		//points by true angle anround p1 and ordering of points by pseudoangle are the
		//same The result is in the range [0, 4) (or error -1). angle relative to the x axis.  */
		//inline float pseudoangle(glm::vec2 p1, glm::vec2 p2) {
		//	float dx, dy, t;
		//	dx = p2.x - p1.x;
		//	dy = p2.y - p1.y;
		//	if ((dx == 0.0) && (dy == 0.0)) {
		//		//return -1; // indicating error
		//		dx += .1;
		//		dy += .1;
		//	}
		//	else {
		//		t = dy / (fabs(dx) + fabs(dy));
		//		/* Now correct for quadrant -- first quadrant: [0,1] */
		//		if (dx < 0.0)
		//			return 2.0 - t;
		//		/* Inside second or third quadrant (1,3)*/
		//		else if (dy < 0.0)
		//			return 4.0 + t;
		//		return t;
		//	}
		//}

		GLuint vertexBuffer = 0;
		GLuint colorBuffer = 0;
		GLuint indexBuffer = 0;
		GLuint normalBuffer = 0;
		GLuint texCoordBuffer = 0;

		void createVertexBuffer() {
			if (vertexBuffer != 0)
				glDeleteBuffers(1, &vertexBuffer);
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
			size_t bufferSize = indices.size() * sizeof(unsigned short);
			glCreateBuffers(1, &indexBuffer);
			glNamedBufferData(indexBuffer, bufferSize, indices.data(), GL_STATIC_DRAW);
		}

		void createNormalBuffer() {
			size_t bufferSize = normals.size() * sizeof(glm::vec3);
			glCreateBuffers(1, &normalBuffer);
			glNamedBufferData(normalBuffer, bufferSize, normals.data(), GL_STATIC_DRAW);
		}

		void createTexCoordBuffer() {
			size_t bufferSize = uvs.size() * sizeof(glm::vec2);
			glCreateBuffers(1, &texCoordBuffer);
			glNamedBufferData(texCoordBuffer, bufferSize, uvs.data(), GL_STATIC_DRAW);
		}
	};
}