#include "SlicedVolume.hpp"

#include "Components/Materials/Surface/UnlitSurface.hpp"
#include "Components/Materials/Wireframe/Wireframe.hpp"
//#include "Components/Materials/Volume/BasicProxyGeoVolume.hpp"

namespace Prefabs {
	int SlicedVolume::count = 0;

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

	inline void intersectBox(glm::vec3 boxmin, glm::vec3 boxmax, glm::vec3 p,
		glm::vec3 dir, float &tmin, float &tmax) {
		glm::vec3 dirfrac;
		if (dir.x == 0.0) dir.x += .0000001;
		if (dir.y == 0.0) dir.y += .0000001;
		if (dir.z == 0.0) dir.z += .0000001;

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
	}

	// n: plane normal, p0 : point on plane, l0: ray origin, l: ray direction, t : output
	inline bool intersectPlane(
		const float3 &n, const float3 &p0, const float3 &l0, const float3 &l,
		float tmin, float tmax, float &t)
	{
		// Ray Plane Intersection
		//    (p0 - l0) . n
		//t = ------------- 
		//        l . n
		// parallel when l.n close to zero


		// assuming vectors are all normalized
		float denom = dot(n, l);
		if (fabs(denom) > 1e-6) {
			float3 p0l0 = p0 - l0;
			t = dot(p0l0, n) / denom;
			if (t >= tmin && t <= tmax)
				return true;
		}

		return false;
	}

	/*! Returns the pseudoangle between the line p1 to (infinity, p1.y) and the
	line from p1 to p2. The pseudoangle has the property that the ordering of
	points by true angle anround p1 and ordering of points by pseudoangle are the
	same The result is in the range [0, 4) (or error -1). */
	/* Shamelessly taken from https://www.opengl.org/discussion_boards/showthread.php/169527-Pseudo-angles */
	inline float pseudoangle(float2 p1, float2 p2) {
		float2 delta = p2 - p1;
		float result;

		if ((delta.x == 0) && (delta.y == 0)) {
			return -1;
		}
		else {
			result = delta.y / (fabs(delta.x) + fabs(delta.y));

			if (delta.x < 0.0) {
				result = 2.0f - result;
			}
			else {
				result = 4.0f + result;
			}

		}

		return result;
	}

	// angle relative to the x axis. 
	inline float pseudoangle2(float2 p1, float2 p2) {
		float dx, dy, t;
		dx = p2.x - p1.x;
		dy = p2.y - p1.y;
		if ((dx == 0.0) && (dy == 0.0)) {
			//return -1; // indicating error
			dx += .1;
			dy += .1;
		}
		else {
			t = dy / (fabs(dx) + fabs(dy));
			/* Now correct for quadrant -- first quadrant: [0,1] */
			if (dx < 0.0)
				return 2.0 - t;
			/* Inside second or third quadrant (1,3)*/
			else if (dy < 0.0)
				return 4.0 + t;
			return t;
		}
	}

	SlicedVolume::SlicedVolume(
		string filename,
		glm::vec3 rawDimensions,
		int bytesPerPixel,
		shared_ptr<Entity> perceivingEntity,
		int samples) : Volume(filename, rawDimensions, bytesPerPixel, samples) // todo, change back to samples
	{
		using namespace Components::Materials;
		this->perceivingEntity = perceivingEntity;

		count++;
		offset = make_float4(0.0, 0.0, 0.0, 0.0);
		
		updateVBO();
		updateImage(filename, rawDimensions, bytesPerPixel);
		if (!volumeLoaded) return;
		//compute2DHistogram();

		/* Add wireframe material */
		//std::shared_ptr<BasicProxyGeoVolumeMaterial> volumeMaterial = dynamic_pointer_cast<BasicProxyGeoVolumeMaterial>(System::MaterialList["VolumeMaterial"]);
		//volumeMaterial->setUniformData(samples, false, volumeTextureID);
		//this->materials.push_back(volumeMaterial);
		
	/*	std::shared_ptr<UnlitSurface> blueSurface = dynamic_pointer_cast<UnlitSurface>(Scene::MaterialList["BlueSurface"]);
		this->materials.push_back(blueSurface);*/

		//std::shared_ptr<WireframeMaterial> redWire = dynamic_pointer_cast<WireframeMaterial>(Scene::MaterialList["RedWire"]);
		//this->materials.push_back(redWire);
	}
	
	void SlicedVolume::updateVBO() {
		using namespace glm;
		using namespace std;

		vec3 pos = perceivingEntity->transform.position;
		vec3 cameraRight = perceivingEntity->transform.right; /* BUG HERE. Right isn't transformed yet */
		vec3 frwd = perceivingEntity->transform.forward;

		pos = vec3(this->transform.ParentToLocalMatrix() * vec4(pos, 1.0));
		cameraRight = vec3(this->transform.ParentToLocalMatrix() * vec4(cameraRight, 0.0));
		frwd = vec3(this->transform.ParentToLocalMatrix() * vec4(frwd, 0.0));

		/* Issue with pseudoangle, requires projecting in a dimension.
			If looking axis aligned, proxy geo might not sort correctly
		*/
		if (frwd.x == 0.0) frwd.x += .000001;
		if (frwd.y == 0.0) frwd.y += .000001;
		if (frwd.z == 0.0) frwd.z += .000001;

		//TEST
		pos = vec3(0.0, 0.0, 0.0);
		//pos = vec3(vec4(pos, 1.0) * this->transform.ParentToLocalMatrix());
		//cameraRight = vec3(vec4(cameraRight, 1.0) * this->transform.ParentToLocalMatrix());
		//frwd = vec3(vec4(frwd, 1.0) * this->transform.ParentToLocalMatrix());
		//
		/* Find intersection with a 1 by 1 by 1 axis aligned cube at the world origin. */
		float tmin, tmax;
		intersectBox(vec3( -.5, -.5, -.5 ), vec3( .5, .5, .5 ),
		vec3( pos.x, pos.y, pos.z ), vec3( frwd.x, frwd.y, frwd.z ), tmin, tmax);
		
		/* Start and end points for the ray */
		vec4 start = vec4(pos + (tmin * frwd), 1);
		vec4 end = vec4(pos + (tmax * frwd), 1);

		/* Vectors to store different points */
		std::vector<vec3> samplePoints;
		std::vector<vec4> centerPoints;
		std::vector<vec3> facePoints;

		const vec3 directions[3] = {
			{ 1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, 0, 1 },
		};
		
		float dAlpha = (sqrt(2.0)) / samples;
		for (int i = 0; i < samples; ++i) {
			vector<pair<vec4, float>> pointsOnCurrentPlane;
			float alpha = i * dAlpha;
			vec4 sPt = (start * alpha) + (end * (1 - alpha));

			/* Add sample point */
			samplePoints.push_back(vec3(sPt.x, sPt.y, sPt.z));

			// Ray Plane Intersection
			//    (p0 - l0) . n
			//t = ------------- 
			//        l . n
			// parallel when l.n close to zero
			float t;
			for (int dim = 0; dim < 3; ++dim) {
				vec3 dir = directions[dim];
				float point[3] = { .5, .5, .5 };
				point[dim] = -.5; // project onto dim.

				for (int p = 0; p < 4; ++p) {
					int temp = p;
					if (dim != 0) {
						point[0] = (temp & 1) ? point[0] : -point[0];
						temp++;
					}
					if (dim != 1) {
						point[1] = (temp & 1) ? point[1] : -point[1];
						temp++;
					}
					if (dim != 2) {
						point[2] = (temp & 1) ? point[2] : -point[2];
						temp++;
					}

					float t;
					if (intersectPlane({ frwd.x, frwd.y, frwd.z }, { sPt.x, sPt.y, sPt.z },
					{ point[0], point[1], point[2] }, { dir.x, dir.y, dir.z }, 0, 1, t))
					{
						vec3 edgePt = { point[0], point[1], point[2] };
						edgePt += dir * t;

						float angle = pseudoangle2({ sPt.x, sPt.z }, { edgePt.x, edgePt.z });
						pointsOnCurrentPlane.push_back({ { edgePt.x, edgePt.y, edgePt.z, 1.0 }, angle });
					}
				}
			}

			// really ghetto bubble sort. fewer than 6 pts, so this should be okay
			for (int n = 0; n < pointsOnCurrentPlane.size(); ++n) {
				for (int n2 = 0; n2 < pointsOnCurrentPlane.size(); n2++) {
					if ((frwd.y > 0 && pointsOnCurrentPlane[n].second < pointsOnCurrentPlane[n2].second) ||
						(frwd.y < 0 && pointsOnCurrentPlane[n].second > pointsOnCurrentPlane[n2].second)) {
						pair<vec4, float> temp = pointsOnCurrentPlane[n];
						pointsOnCurrentPlane[n] = pointsOnCurrentPlane[n2];
						pointsOnCurrentPlane[n2] = temp;
					}
				}
			}

			/* Compute centroid */
			vec4 centroid(0.0);
			for (int n = 0; n < pointsOnCurrentPlane.size(); ++n) {
				centroid += pointsOnCurrentPlane.at(n).first;
			}
			centroid /= pointsOnCurrentPlane.size();

			/*for (int j = 0; j < pointsOnPlane.size(); ++j) {
				edgePoints.push_back(pointsOnPlane[j].first);
				edgePoints.push_back({ 0.0, 1.0, 0.0, 1.0 });
			}*/
			/* Add triangles to draw. Note, equal number of trangles as there are border points */
			for (int j = 0; j < pointsOnCurrentPlane.size(); ++j) {
				vec4 p1 = pointsOnCurrentPlane.at(j).first;
				vec4 p2 = pointsOnCurrentPlane.at((j == pointsOnCurrentPlane.size() - 1) ? 0 : j + 1).first;

				facePoints.push_back({ p1.x, p1.y, p1.z});
				facePoints.push_back({ p2.x, p2.y, p2.z});
				facePoints.push_back({ centroid.x, centroid.y, centroid.z});
				//facePoints.push_back(vec4(0.0, 1.0, 0.0, 1.0) * (alpha));
				//facePoints.push_back(vec4(0.0, 1.0, 0.0, ) * (alpha));
				//facePoints.push_back(vec4(0.0, 1.0, 0.0, ) * (alpha));
			}
		}

		/* For sample points */
		numSamplePoints = samplePoints.size();
		glDeleteBuffers(1, &samplePointsVBO);
		glGenBuffers(1, &samplePointsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, samplePointsVBO);
		glBufferData(GL_ARRAY_BUFFER, numSamplePoints * sizeof(glm::vec3), samplePoints.data(), GL_DYNAMIC_DRAW);

		/* For edge points/faces */
		numPoints = facePoints.size();
		glDeleteBuffers(1, &pointsVBO);
		glGenBuffers(1, &pointsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
		glBufferData(GL_ARRAY_BUFFER, numPoints * sizeof(glm::vec3), facePoints.data(), GL_DYNAMIC_DRAW);

		//std::shared_ptr<UnlitSurface> blueSurface = dynamic_pointer_cast<UnlitSurface>(Scene::MaterialList["BlueSurface"]);
		//blueSurface->setAttributeData(pointsVBO, numPoints);

		/*std::shared_ptr<WireframeMaterial> redWire = dynamic_pointer_cast<WireframeMaterial>(Scene::MaterialList["RedWire"]);
		redWire->setAttributeData(samplePointsVBO, numSamplePoints);*/
		using namespace Components::Materials;

		//std::shared_ptr<BasicProxyGeoVolumeMaterial> volumeMaterial = dynamic_pointer_cast<BasicProxyGeoVolumeMaterial>(System::MaterialList["VolumeMaterial"]);
		//volumeMaterial->setAttributeData(pointsVBO, numPoints);

	}
	
	void SlicedVolume::prerender(glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
		updateVBO();
	}
	
	void SlicedVolume::render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
		updateVBO();
		Entity::render(model, view, projection);
		//updateVAO();
		//glm::mat4 finalMatrix = projection * parent_matrix * transform.LocalToParentMatrix();

		//Shaders::pointProgram->use();


		///* Edge points*/
		//if (renderEdgePoints || renderFaces) {
		//	glBindVertexArray(edgePointsVAO);
		//	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
		//	glUniformMatrix4fv(Shaders::pointProgram->matrix_id, 1, 0, &(finalMatrix[0].x));
		//	glUniform1f(Shaders::pointProgram->pointSize_id, 10.0);
		//	if (renderEdgePoints)
		//		glDrawArrays(GL_POINTS, 0, numPoints / sizeof(float4));
		//	if (renderFaces)
		//		glDrawArrays(GL_TRIANGLES, 0, numPoints / sizeof(float4));
		//}

		///* Sample points */
		//if (renderSamplePoints) {
		//	glBindVertexArray(samplePointsVAO);
		//	glBindBuffer(GL_ARRAY_BUFFER, samplePointsVBO);
		//	glUniformMatrix4fv(Shaders::pointProgram->matrix_id, 1, 0, &(finalMatrix[0].x));
		//	glUniform1f(Shaders::pointProgram->pointSize_id, 10.0);
		//	glDrawArrays(GL_POINTS, 0, samplePointsVBOSize / sizeof(float4));
		//	glDrawArrays(GL_LINE_STRIP, 0, samplePointsVBOSize / sizeof(float4));
		//}

		//if (renderSlices) {
		//	Shaders::slicedVolProgram->use();
		//	glBindVertexArray(pointsVBO);
		//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//	glUniformMatrix4fv(Shaders::slicedVolProgram->matrix_id, 1, 0, &(finalMatrix[0].x));
		//	glUniform1i(Shaders::slicedVolProgram->samples_id, samples);

		//	glActiveTexture(GL_TEXTURE0);
		//	glBindTexture(GL_TEXTURE_3D, volumeTextureID);

		//	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, (interpolate) ? GL_LINEAR : GL_NEAREST);
		//	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, (interpolate) ? GL_LINEAR : GL_NEAREST);

		//	glUniform1i(Shaders::slicedVolProgram->texture0_id, 0);

		//	glActiveTexture(GL_TEXTURE1);
		//	glBindTexture(GL_TEXTURE_2D, transferFunction->textureID);
		//	glUniform1i(Shaders::slicedVolProgram->texture1_id, 1);


		//	glDrawArrays(GL_TRIANGLES, 0, numPoints / sizeof(float4));
		//	print_gl_error();
		//}
	}

	void SlicedVolume::handleKeys()
	{
		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_N)) {
			interpolate = false;
		}
		else if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_L)) {
			interpolate = true;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_W)) {
			renderSlices = true;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_S)) {
			renderSlices = false;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_Y)) {
			samples = 32;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_U)) {
			samples = 64;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_I)) {
			samples = 128;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_O)) {
			samples = 256;
		}

		if (glfwGetKey(GLDK::DefaultWindow, GLFW_KEY_P)) {
			samples = 1024;
		}
	}

	void SlicedVolume::update() {
		//this->transform.AddRotation(glm::angleAxis(0.01f, glm::vec3(0.0, 0.0, 1.0)));
		//this->transform.AddRotation(glm::angleAxis(0.001f, glm::vec3(1.0, 1.0, 1.0)));


		handleKeys();
	}
}
